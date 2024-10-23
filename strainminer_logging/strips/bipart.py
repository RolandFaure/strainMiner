"""Bi-partition views module."""

from __future__ import annotations

from enum import Enum

from strainminer_logging.binmatrix import BinMatrixStats


class BiPartClass(Enum):
    """Bi-part class."""

    ZERO = 0
    ONE = 1

    def complement(self) -> BiPartClass:
        """Return the complement of the bi-part class."""
        return BiPartClass(1 - self.value)


class BiPartStats:
    """Bi-partition views class."""

    KEY_INDEX: str = "index"
    KEY_BIPART_CLASS: str = "class"
    KEY_BINMATRIX_STATS: str = "binmatrix_stats"

    @classmethod
    def from_dict(cls, d: dict) -> BiPartStats:
        """Convert dict to BiPartViews.

        Parameters
        ----------
        d : dict
            Dictionnary of view

        Returns
        -------
        BiPartViews
            BiPartViews

        """
        return cls(
            d[cls.KEY_INDEX],
            BiPartClass(d[cls.KEY_BIPART_CLASS]),
            BinMatrixStats.from_dict(d[cls.KEY_BINMATRIX_STATS]),
        )

    def __init__(
        self,
        index: int,
        bipart_class: BiPartClass,
        binmatrix_stats: BinMatrixStats,
    ) -> None:
        """Initialize."""
        self.__index: int = index
        self.__bipart_class: BiPartClass = bipart_class
        self.__binmatrix_stats: BinMatrixStats = binmatrix_stats

    def index(self) -> int:
        """Return the index.

        Returns
        -------
        int
            Index

        """
        return self.__index

    def bipart_class(self) -> BiPartClass:
        """Return the bipart class.

        Returns
        -------
        BiPartClass
            Bipart class

        """
        return self.__bipart_class

    def binmatrix_stats(self) -> BinMatrixStats:
        """Return the binmatrix stats.

        Returns
        -------
        BinMatrixStats
            Binmatrix stats

        """
        return self.__binmatrix_stats

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_INDEX: self.__index,
            self.KEY_BIPART_CLASS: self.__bipart_class.value,
            self.KEY_BINMATRIX_STATS: self.__binmatrix_stats.to_dict(),
        }

    def __str__(self) -> str:
        """Convert to string.

        Returns
        -------
        str
            String

        """
        return "\n".join(
            [
                f"index: {self.__index}",
                f"bipart class: {self.__bipart_class.value}",
                "",
                "binmatrix stats",
                "---------------",
                str(self.__binmatrix_stats),
                "",
            ],
        )
