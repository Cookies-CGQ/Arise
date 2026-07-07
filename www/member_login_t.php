<?php
/**
 * 第三方账号验证接口
 *
 * POST /member_login_t.php
 * 参数: account=xxx&password=xxx
 *
 * 返回 JSON:
 *   returncode: 0=成功, 2=账号不存在, 3=密码错误
 */

header('Content-Type: application/json');

// ============================================================
// 预注册的测试账号 (account => password)
// 机器人工具中使用的密码是 "e10adc3949ba59abbe56e057f20f883e" (MD5 of "123456")
// ============================================================
$accounts = [
    'player1' => 'e10adc3949ba59abbe56e057f20f883e',
    'admin'   => 'e10adc3949ba59abbe56e057f20f883e',
];

// ============================================================
// 动态账号支持: 任何以 "bot" 或 "test" 为前缀的账号自动注册
// 密码为 "e10adc3949ba59abbe56e057f20f883e"
// ============================================================
function getExpectedPassword($account) {
    $prefixes = ['bot', 'test', 'player'];
    foreach ($prefixes as $prefix) {
        if (str_starts_with($account, $prefix)) {
            return 'e10adc3949ba59abbe56e057f20f883e';
        }
    }
    return null;
}

// ============================================================
// 获取请求参数
// ============================================================
$account  = $_POST['account']  ?? '';
$password = $_POST['password'] ?? '';

if (empty($account)) {
    echo json_encode(['returncode' => 2]); // 账号不存在
    exit;
}

// ============================================================
// 验证
// ============================================================

// 1. 查预注册表
if (isset($accounts[$account])) {
    if ($accounts[$account] === $password) {
        echo json_encode(['returncode' => 0]); // OK
    } else {
        echo json_encode(['returncode' => 3]); // 密码错误
    }
    exit;
}

// 2. 动态账号
$expectedPwd = getExpectedPassword($account);
if ($expectedPwd !== null) {
    if ($expectedPwd === $password) {
        echo json_encode(['returncode' => 0]); // OK
    } else {
        echo json_encode(['returncode' => 3]); // 密码错误
    }
    exit;
}

// 3. 账号不存在
echo json_encode(['returncode' => 2]);
