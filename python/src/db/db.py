import aiosqlite
import asyncio
from contextlib import asynccontextmanager
from dataclasses import dataclass


@dataclass
class Project:
    name: str
    crc: int


@dataclass
class ProjectData:
    data: bytes


class DB:
    def __init__(self, config):
        self._config = config
        loop = asyncio.get_event_loop()
        coroutine = self._create_db()
        loop.run_until_complete(coroutine)

    @asynccontextmanager
    async def _get_cursor(self):
        async with aiosqlite.connect(self._config.database.file) as connection:
            async with connection.cursor() as cursor:
                try:
                    yield cursor
                    await connection.commit()
                except Exception:
                    await connection.rollback()
                    raise

    async def _create_db(self):
        async with self._get_cursor() as cursor:
            await cursor.execute("PRAGMA foreign_keys = ON")

            await cursor.executescript("""
                CREATE TABLE IF NOT EXISTS project (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    vin TEXT NOT NULL,
                    name TEXT NOT NULL,
                    crc INTEGER NOT NULL
                );
                CREATE TABLE IF NOT EXISTS project_data (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    project_id INTEGER NOT NULL,
                    content BLOB,
                    FOREIGN KEY(project_id) REFERENCES project(id) ON DELETE CASCADE
                );
            """)

            await cursor.executescript("""
                CREATE INDEX IF NOT EXISTS idx_project_data_project_id ON project_data(project_id);
            """)

            await cursor.execute("insert into project(vin, name, crc) values (?, ?, ?)", ("myvin", "test1", 0x1011))
            await cursor.execute("insert into project(vin, name, crc) values (?, ?, ?)", ("myvin", "test2", 0x1012))

    async def get_available_projects(self, vin: str) -> Project:
        async with self._get_cursor() as cursor:
            await cursor.execute("select name, crc from project where vin = ?", (vin,))
            rows = await cursor.fetchall()
            return [Project(name=row[0], crc=row[1]) for row in rows]

    async def get_project_data(self, vin: str, project_name: str) -> ProjectData:
        async with self._get_cursor() as cursor:
            await cursor.execute(
                "select content from project_data as pd inner join project as p on pd.project_id = p.id where p.vin = ? and p.name = ?",
                (vin, project_name))
            rows = await cursor.fetchone()
            for data in rows:
                return ProjectData(data=rows[0])

            return None
