# 数据库表结构文档

> 数据库名称：`parking_system`
> 字符集：`utf8mb4`
> 存储引擎：`InnoDB`

---

## 1. User 表（用户表）

存储系统用户信息，支持管理员和普通用户角色。

| 字段名 | 类型 | 约束 | 默认值 | 说明 |
|--------|------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 用户ID |
| username | VARCHAR(50) | UNIQUE, NOT NULL | - | 用户名 |
| password | VARCHAR(64) | NOT NULL | - | 密码（SHA256加密） |
| telephone | VARCHAR(11) | - | '' | 手机号 |
| truename | VARCHAR(50) | - | '' | 真实姓名 |
| role | VARCHAR(10) | - | '' | 角色（admin/user） |
| create_at | DATETIME | - | CURRENT_TIMESTAMP | 创建时间 |
| updated_at | DATETIME | - | CURRENT_TIMESTAMP ON UPDATE | 更新时间 |

**索引：**
- `idx_username` - username 字段

```sql
CREATE TABLE IF NOT EXISTS User(
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
    password VARCHAR(64) NOT NULL COMMENT '密码(SHA256)',
    telephone VARCHAR(11) DEFAULT '' COMMENT '手机号',
    truename VARCHAR(50) DEFAULT '' COMMENT '真实姓名',
    role VARCHAR(10) DEFAULT '' COMMENT '角色(admin/user)',
    create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_username(username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';
```

---

## 2. CAR 表（车辆记录表）

记录车辆的进出信息和停车费用。

| 字段名 | 类型 | 约束 | 默认值 | 说明 |
|--------|------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 记录ID |
| license_plate | VARCHAR(20) | NOT NULL | - | 车牌号 |
| check_in_time | DATETIME | NOT NULL | - | 入库时间 |
| check_out_time | DATETIME | - | NULL | 出库时间 |
| fee | DECIMAL(10,2) | - | NULL | 停车费用 |
| location | VARCHAR(50) | NOT NULL | - | 停车位置 |
| create_at | DATETIME | - | CURRENT_TIMESTAMP | 记录创建时间 |

**索引：**
- `idx_license_plate` - license_plate 字段
- `idx_check_in_time` - check_in_time 字段
- `idx_location` - location 字段

```sql
CREATE TABLE IF NOT EXISTS CAR(
    id INT PRIMARY KEY AUTO_INCREMENT,
    license_plate VARCHAR(20) NOT NULL COMMENT '车牌号',
    check_in_time DATETIME NOT NULL COMMENT '入库时间',
    check_out_time DATETIME DEFAULT NULL COMMENT '出库时间',
    fee DECIMAL(10,2) DEFAULT NULL COMMENT '停车费用',
    location VARCHAR(50) NOT NULL COMMENT '停车位置',
    create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
    INDEX idx_license_plate (license_plate),
    INDEX idx_check_in_time (check_in_time),
    INDEX idx_location (location)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='车辆记录表';
```

---

## 3. PARKING 表（停车场记录表）

存储停车场的基本信息和实时状态。

| 字段名 | 类型 | 约束 | 默认值 | 说明 |
|--------|------|------|--------|------|
| P_id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 停车场ID |
| P_name | VARCHAR(100) | UNIQUE, NOT NULL | - | 停车场名称 |
| P_now_count | INT | - | 0 | 现有车辆数 |
| P_all_count | INT | NOT NULL | - | 总车位数 |
| P_fee | DECIMAL(10,2) | NOT NULL | - | 每小时费用 |
| create_at | DATETIME | - | CURRENT_TIMESTAMP | 创建时间 |

```sql
CREATE TABLE IF NOT EXISTS PARKING(
    P_id INT PRIMARY KEY AUTO_INCREMENT,
    P_name VARCHAR(100) UNIQUE NOT NULL COMMENT '停车场名称',
    P_now_count INT DEFAULT 0 COMMENT '现有车辆数',
    P_all_count INT NOT NULL COMMENT '总车位数',
    P_fee DECIMAL(10,2) NOT NULL COMMENT '每小时费用',
    create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='停车场记录表';
```

---

## 4. ER 关系图

```
┌─────────────┐       ┌─────────────┐       ┌─────────────┐
│    User     │       │   PARKING   │       │     CAR     │
├─────────────┤       ├─────────────┤       ├─────────────┤
│ id (PK)     │       │ P_id (PK)   │       │ id (PK)     │
│ username    │       │ P_name (UQ) │       │ license_plate│
│ password    │       │ P_now_count │       │ check_in_time│
│ telephone   │       │ P_all_count │       │ check_out    │
│ truename    │       │ P_fee       │       │ fee          │
│ role        │       │ create_at   │       │ location     │
│ create_at   │       └─────────────┘       │ create_at    │
│ updated_at  │                             └──────────────┘
└─────────────┘
```

---

## 5. 数据流说明

| 操作 | 影响的表 | 说明 |
|------|----------|------|
| 用户注册 | User | 插入新用户记录 |
| 车辆入场 | CAR | 插入车辆记录 |
| 车辆出场 | CAR | 更新出库时间和费用 |
