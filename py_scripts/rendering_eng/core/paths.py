

from pathlib import Path
import os
import sys

ENGINE_ROOT = Path(__file__).resolve().parent.parent

class PathUtil:
    @staticmethod
    def fp(relative_path: str) -> Path:
        p = Path(relative_path)
        if p.is_absolute():
            return p
        return (ENGINE_ROOT / p).resolve()

