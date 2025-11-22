from typing import Optional
from .data_provider_base import DataProviderBase
from db import DB
from models import Project, ProjectData, Log, LogData


class DBProvider(DataProviderBase):
    def __init__(self, db: DB):
        self._db = db

    async def get_available_projects(self, vin: str) -> list[Project]:

        return await self._db.get_available_projects(vin)

    async def get_project_data(self, vin: str, project_name: str) -> Optional[ProjectData]:
        return await self._db.get_project_data(vin, project_name)

    async def get_available_logs(self, vin: str) -> list[Log]:
        return await self._db.get_available_logs(vin)

    async def get_log_data(self, vin: str, log_name: str) -> Optional[LogData]:
        return await self._db.get_log_data(vin, log_name)

    async def put_project(self, vin: str, project: Project, project_data: ProjectData):
        await self._db.put_project(vin, project, project_data)

    async def put_log(self, vin: str, log: Log, log_data: LogData):
        await self._db.put_logs(vin, log, log_data)
