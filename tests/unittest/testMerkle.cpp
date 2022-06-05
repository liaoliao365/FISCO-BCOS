#include <bcos-crypto/hasher/OpenSSLHasher.h>
#include <bcos-tool/Merkle.h>
#include <bcos-tool/MerkleSerialization.h>
#include <bcos-utilities/FixedBytes.h>
#include <boost/algorithm/hex.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <future>
#include <iterator>
#include <ostream>
#include <random>
#include <stdexcept>

using HashType = std::array<std::byte, 32>;

namespace std
{
std::ostream& operator<<(std::ostream& stream, const HashType& hash)
{
    std::string hex;
    boost::algorithm::hex_lower(
        (char*)hash.data(), (char*)hash.data() + hash.size(), std::back_inserter(hex));
    std::string_view view{hex.data(), 8};
    stream << view;
    return stream;
}
}  // namespace std

namespace bcos::test
{
struct TestBinaryMerkleTrieFixture
{
    std::array<HashType, 128> hashes;

    TestBinaryMerkleTrieFixture()
    {
        crypto::hasher::openssl::OpenSSL_SHA3_256_Hasher hasher;
        std::mt19937 prng(std::random_device{}());

        for (auto& element : hashes)
        {
            hasher.update(prng());
            element = hasher.final();
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(TestBinaryMerkleTrie, TestBinaryMerkleTrieFixture)

template <size_t width>
void testFixedWidthMerkle(bcos::tool::merkle::InputRange<HashType> auto const& inputHashes)
{
    HashType emptyHash;
    emptyHash.fill(std::byte(0));
    auto seed = std::random_device{}();

    for (auto count = 0lu; count < std::size(inputHashes); ++count)
    {
        std::span<HashType const> hashes(inputHashes.data(), count);

        bcos::tool::merkle::Merkle<bcos::crypto::hasher::openssl::OpenSSL_SHA3_256_Hasher, HashType,
            width>
            trie;
        BOOST_CHECK_THROW(
            trie.import(std::vector<HashType>{}), boost::wrapexcept<std::invalid_argument>);

        if (count == 0)
        {
            BOOST_CHECK_THROW(
                trie.import(std::as_const(hashes)), boost::wrapexcept<std::invalid_argument>);
        }
        else
        {
            BOOST_CHECK_NO_THROW(trie.import(std::as_const(hashes)));

            BOOST_CHECK_THROW(
                trie.generateProof(emptyHash), boost::wrapexcept<std::invalid_argument>);

            for (auto& hash : hashes)
            {
                auto proof = trie.generateProof(hash);

                BOOST_CHECK(trie.verifyProof(proof, hash, trie.root()));
                BOOST_CHECK(!trie.verifyProof(proof, emptyHash, trie.root()));

                auto dis = std::uniform_int_distribution(0lu, proof.hashes.size() - 1);
                std::mt19937 prng{seed};
                proof.hashes[dis(prng)] = emptyHash;

                BOOST_CHECK(!trie.verifyProof(proof, hash, trie.root()));

                proof.hashes.clear();
                BOOST_CHECK_THROW(trie.verifyProof(proof, emptyHash, trie.root()),
                    boost::wrapexcept<std::invalid_argument>);
            }
        }
    }
}

template <size_t i>
constexpr void loopWidthTest(bcos::tool::merkle::InputRange<HashType> auto const& inputHashes)
{
    testFixedWidthMerkle<i>(inputHashes);

    if constexpr (i > 2)
    {
        loopWidthTest<i - 1>(inputHashes);
    }
}

BOOST_AUTO_TEST_CASE(merkle)
{
    loopWidthTest<32>(hashes);
}

BOOST_AUTO_TEST_CASE(serialize)
{
    using MerkleType =
        bcos::tool::merkle::Merkle<bcos::crypto::hasher::openssl::OpenSSL_SHA3_256_Hasher, HashType,
            4>;
    MerkleType trie1;
    BOOST_CHECK_NO_THROW(trie1.import(hashes));
    std::cout.operator<<(trie1) << std::endl;
}

BOOST_AUTO_TEST_CASE(performance) {}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace bcos::test