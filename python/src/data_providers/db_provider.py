from .data_provider_base import DataProviderBase
from db import DB
from models import Project, ProjectData


class DBProvider(DataProviderBase):
    def __init__(self, db: DB):
        self._db = db

    def get_available_projects(self, vin: str) -> list[Project]:
        return self._db.get_available_projects(vin)

    def get_project_data(self, vin: str, project_name: str) -> ProjectData:
        return self._db.get_project_data(vin, project_name)
