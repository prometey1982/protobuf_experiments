from abc import ABC, abstractmethod
from models import Project, ProjectData

class DataProviderBase(ABC):
    @abstractmethod
    def get_available_projects(self, vin: str) -> list[Project]:
        pass

    @abstractmethod
    def get_project_data(self, vin: str, project_name: str) -> ProjectData:
        pass
