#pragma once

#include <sstream>
#include "libserver/common.h"
#include "libserver/component.h"
#include "libserver/system.h"

// 挂载在 player 的组件，把一次查出来的角色列表（Proto::PlayerList）按玩家 SN 拆分成独立的序列化缓冲区，缓存在内存里，供后续按需取出
class PlayerComponentProtoList :public Component<PlayerComponentProtoList>, public IAwakeFromPoolSystem<>
{
public:
    void Awake() override {}
    void BackToPool() override;

    // 遍历列表里每个player，各自SerializeToOstream 到一个独立新建的 stringstream，按 sn 存入 map
    void Parse(Proto::PlayerList& proto);
    // 按 sn 取出对应角色的序列化流，返回指针
    std::stringstream* GetProto(uint64 sn);

private:
    // playerSN : stringstream(单个Proto::Player的protobuf二进制序列化结果)
    std::map<uint64, std::stringstream*> _protos;
};