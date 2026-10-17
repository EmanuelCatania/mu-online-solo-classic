# MySQL 8.4 LTS migration

The stack uses MySQL 8.4 LTS. When upgrading an existing installation from MySQL 5.7 or 8.0, use a logical backup and restore instead of reusing the old data volume.

## 1. Export data

With the existing stack still running, create a backup that contains all application tables. The new container creates the schema and stored procedures from the repository before the import.

```bash
mkdir -p backups
docker compose exec -T mysql sh -c 'mysqldump -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" --single-transaction --no-tablespaces --skip-routines --skip-triggers "$MYSQL_DATABASE"' > backups/muonline97-before-mysql84.sql
test -s backups/muonline97-before-mysql84.sql
```

Keep that file until the migration has been verified.

## 2. Recreate only the MySQL volume

Stop the stack, identify the exact MySQL volume, and remove only that volume after verifying the backup:

```bash
docker compose down
docker volume ls --format '{{.Name}}' | grep '_mu_mysql$'
docker volume rm EXACT_PROJECT_mu_mysql
```

## 3. Start MySQL 8.4 and restore

```bash
docker compose up -d --build mysql
docker compose exec -T mysql sh -c 'mysql -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" "$MYSQL_DATABASE"' < backups/muonline97-before-mysql84.sql
docker compose up -d --build
```

## 4. Verify

Confirm that accounts, characters, guilds, warehouses, web panel and editor work as expected. Do not remove the backup until the verification is complete.
