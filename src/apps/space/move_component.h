#pragma once

#include <queue>
#include "libserver/component.h"
#include "libserver/system.h"
#include "libserver/vector3.h"

class PlayerComponentLastMap;

// 当前移动段的方向和目标
struct MoveVector3
{
    // 根据起点和终点计算单位方向向量
	void Update(Vector3 target, Vector3 position);

	Vector3 Target = Vector3::Zero; // 本段目标点
	float ScaleX = 0;               // 方向单位向量 x 分量
	float ScaleZ = 0;               // 方向单位向量 z 分量
};

// 移动组件：space 侧 Player 上的按需组件:玩家发出第一条移动指令才挂上,最后一段路走完就自动卸载回池
// 1、存储客户端上传的路点队列(数据面)
// 2、推进服务器上的玩家坐标——按固定速度沿路点逐段插值(时间面)
// World::HandleMove --注入路点--> MoveComponent --帧推进--> MoveSystem
class MoveComponent :public Component<MoveComponent>, public IAwakeFromPoolSystem<>
{
public:
	void Awake() override;
	void BackToPool() override;

    // 注入路点
	void Update(std::queue<Vector3> targets, Vector3 curPosition);
	// 帧推进
    bool Update(float timeElapsed, PlayerComponentLastMap* pLastMap, const float speed);

private:
	std::queue<Vector3> _targets; // 路点队列
	MoveVector3 _vector3;         // 当前段状态（目标点）
};