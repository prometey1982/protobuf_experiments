import yaml
from pathlib import Path
from typing import Any, Dict


class Config:
    def __init__(self, config_path: str = "config.yaml"):
        self.config_path = Path(config_path)
        self._data = self._load_config()

        # Преобразуем корневой словарь в атрибуты
        for key, value in self._data.items():
            if isinstance(value, dict):
                setattr(self, key, DictAsObject(value))
            else:
                setattr(self, key, value)

    def _load_config(self) -> Dict[str, Any]:
        with open(self.config_path, 'r') as file:
            return yaml.safe_load(file)

    def get(self, key: str, default: Any = None) -> Any:
        """Безопасное получение значения с точечной нотацией"""
        keys = key.split('.')
        value = self._data

        for k in keys:
            if isinstance(value, dict) and k in value:
                value = value[k]
            else:
                return default
        return value


class DictAsObject:
    """Класс для преобразования словаря в объект с атрибутами"""

    def __init__(self, data: Dict[str, Any]):
        for key, value in data.items():
            if isinstance(value, dict):
                setattr(self, key, DictAsObject(value))
            else:
                setattr(self, key, value)
