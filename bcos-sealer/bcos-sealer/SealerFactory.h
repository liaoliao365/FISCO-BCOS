/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file SealerFactory.h
 * @author: yujiechen
 * @date: 2021-05-20
 */
#pragma once
#include "Sealer.h"
#include "SealerConfig.h"
#include "VRFBasedSealer.h"
#include <bcos-tool/NodeConfig.h>
namespace bcos
{
namespace sealer
{
class SealerFactory
{
public:
    using Ptr = std::shared_ptr<SealerFactory>;
    SealerFactory(bcos::tool::NodeConfig::Ptr _nodeConfig,
        bcos::protocol::BlockFactory::Ptr _blockFactory, bcos::txpool::TxPoolInterface::Ptr _txpool,
        bcos::tool::NodeTimeMaintenance::Ptr _nodeTimeMaintenance,
        bcos::crypto::KeyPairInterface::Ptr _key);

    virtual ~SealerFactory() = default;
    Sealer::Ptr createSealer();
    VRFBasedSealer::Ptr createVRFBasedSealer();

protected:
    std::string m_groupId{};
    std::string m_chainId{};
    bcos::protocol::BlockFactory::Ptr m_blockFactory;
    bcos::txpool::TxPoolInterface::Ptr m_txpool;
    unsigned m_minSealTime;
    bcos::tool::NodeTimeMaintenance::Ptr m_nodeTimeMaintenance;
    bcos::crypto::KeyPairInterface::Ptr m_keyPair;
};
}  // namespace sealer
}  // namespace bcos