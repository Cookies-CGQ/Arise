<?php
/**
 * member_login_t.php - Arise 游戏服务器的第三方账号验证接口
 *
 * 由 login 服务调用（res/engine.yaml -> login.url_login）：
 *   GET /member_login_t.php?account=<账号>&password=<密码>
 *
 * 必须返回 HTTP 200 + JSON：
 *   {"returncode": 0}  验证成功
 *   {"returncode": 2}  账号不存在
 *   {"returncode": 3}  密码错误
 *   （其他值按未知错误处理；非 200 状态码按超时处理）
 */

header('Content-Type: application/json; charset=utf-8');

// 统一响应出口：必须带 Content-Length（服务端解析依赖这个头）
function respond($code) {
    $body = json_encode(array('returncode' => $code));
    header('Content-Length: ' . strlen($body));
    echo $body;
    exit;
}

$account  = isset($_GET['account'])  ? trim($_GET['account'])  : '';
$password = isset($_GET['password']) ? $_GET['password']       : '';

// 参数缺失，视为账号不存在
if ($account === '' || $password === '') {
    respond(2);
}

// ============================================================
// 方式一：本地测试用，写死一个测试账号（开箱即用）
// 注意：robots 工具发的密码是 123456 的 MD5，所以两种都接受
// ============================================================
if ($account === 'test1' && in_array($password, array('123456', md5('123456')), true)) {
    respond(0);
}
if ($account === 'test1') {
    respond(3); // 密码错误
}
respond(2);     // 账号不存在

// ============================================================
// 方式二：对接自有账号库（MySQL，PDO 扩展）
// 使用前先建表：
//   CREATE TABLE `users` (
//     `account`  varchar(64) NOT NULL,
//     `password` varchar(255) NOT NULL,  -- 存 password_hash() 的哈希
//     PRIMARY KEY (`account`)
//   ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
// ============================================================
/*
$db = new PDO(
    'mysql:host=127.0.0.1;port=3306;dbname=your_user_db;charset=utf8',
    'your_db_user',
    'your_db_password',
    array(PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION)
);

$stmt = $db->prepare('SELECT `password` FROM `users` WHERE `account` = ?');
$stmt->execute(array($account));
$row = $stmt->fetch(PDO::FETCH_ASSOC);

// 账号不存在
if ($row === false) {
    echo json_encode(array('returncode' => 2));
    exit;
}

// 密码校验：数据库存的是 password_hash() 哈希则用 password_verify；
// 如果只是简单存明文（仅限内部测试），直接字符串比较
if (!password_verify($password, $row['password'])) {
    echo json_encode(array('returncode' => 3));
    exit;
}

// 验证成功
echo json_encode(array('returncode' => 0));
*/
