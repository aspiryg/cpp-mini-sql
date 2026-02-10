# Mini SQL Engine

A basic SQL engine built from scratch in C++ as a learning project.

## Features

- **CREATE TABLE** — define tables with `INT` and `TEXT` columns
- **INSERT INTO** — add rows to tables
- **SELECT** — query data with column selection or `*`
- **WHERE** — filter with `=`, `!=`, `<`, `>`, `<=`, `>=`, plus `AND`/`OR`
- Pretty-printed ASCII table output
- Interactive REPL with error messages

## Architecture

```
SQL string → Tokenizer → Tokens → Parser → AST → Executor → Result
                                                      ↕
                                                  Database
                                              (in-memory storage)
```

| File              | Purpose                                       |
| ----------------- | --------------------------------------------- |
| `src/tokenizer.h` | Lexer — breaks SQL text into tokens           |
| `src/ast.h`       | AST node definitions (statements/expressions) |
| `src/parser.h`    | Recursive-descent parser                      |
| `src/database.h`  | In-memory table storage                       |
| `src/executor.h`  | Query execution engine + result formatting    |
| `src/main.cpp`    | REPL entry point                              |

## Build

```bash
cmake -B build
cmake --build build
```

## Run

```bash
./build/mini-sql        # Linux/Mac
.\build\Debug\mini-sql  # Windows
```

## Example Session

```
sql> CREATE TABLE users (id INT, name TEXT, age INT);
Table 'users' created

sql> INSERT INTO users VALUES (1, 'Alice', 30);
1 row inserted into 'users'

sql> INSERT INTO users VALUES (2, 'Bob', 25);
1 row inserted into 'users'

sql> INSERT INTO users VALUES (3, 'Charlie', 35);
1 row inserted into 'users'

sql> SELECT * FROM users;
+----+---------+-----+
| id | name    | age |
+----+---------+-----+
| 1  | Alice   | 30  |
| 2  | Bob     | 25  |
| 3  | Charlie | 35  |
+----+---------+-----+
3 row(s) returned

sql> SELECT name, age FROM users WHERE age > 25;
+---------+-----+
| name    | age |
+---------+-----+
| Alice   | 30  |
| Charlie | 35  |
+---------+-----+
2 row(s) returned

sql> SELECT * FROM users WHERE age >= 30 AND name = 'Alice';
+----+-------+-----+
| id | name  | age |
+----+-------+-----+
| 1  | Alice | 30  |
+----+-------+-----+
1 row(s) returned
```

## 🤝 Contributing

This is a learning project! Feel free to:
- Add new features
- Improve error messages
- Add more comprehensive validation
- Optimize performance
- Add tests

## 📄 License

This project is for educational purposes. Use it freely to learn!
