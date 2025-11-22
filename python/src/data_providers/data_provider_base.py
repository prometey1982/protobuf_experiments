from abc import ABC, abstractmethod
from typing import Optional

from models import Project, ProjectData, Log, LogData

class DataProviderBase(ABC):
    @abstractmethod
    async def get_available_projects(self, vin: str) -> list[Project]:
        pass

    @abstractmethod
    async def get_project_data(self, vin: str, project_name: str) -> Optional[ProjectData]:
        pass

    @abstractmethod
    async def get_available_logs(self, vin: str) -> list[Log]:
        pass

    @abstractmethod
    async def get_log_data(self, vin: str, log_name: str) -> Optional[LogData]:
        pass

    @abstractmethod
    async def put_project(self, vin: str, project: Project, project_data: ProjectData):
        pass

    @abstractmethod
    async def put_log(self, vin: str, log: Log, log_data: LogData):
        pass
