"""strip views module."""

from __future__ import annotations

import yaml  # type: ignore[import-untyped]

try:
    from yaml import CDumper as Dumper
except ImportError:
    from yaml import Dumper

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from pathlib import Path

from typing import Iterable, Iterator

from strainminer_logging.binmatrix import BinMatrixStats
from strainminer_logging.strips.bipart import BiPartStats


class QBCStripStats:
    """QBC strip views class."""

    KEY_INDEX: str = "index"
    KEY_NUMBER_OF_COLUMNS: str = "number_of_columns"
    KEY_BIPART_STATS: str = "bipart_stats"
    KEY_UNASSIGNED_READS: str = "unassigned_reads"

    @classmethod
    def from_dict(cls, d: dict) -> QBCStripStats:
        """Convert dict to StripStats.

        Parameters
        ----------
        d : dict
            Dictionnary of view

        Returns
        -------
        StripStats
            StripStats

        """
        return cls(
            d[cls.KEY_INDEX],
            d[cls.KEY_NUMBER_OF_COLUMNS],
            (
                BiPartStats.from_dict(bipart_stats)
                for bipart_stats in d[cls.KEY_BIPART_STATS]
            ),
            (
                UnassignedReadStats.from_dict(unassigned_read_stats)
                for unassigned_read_stats in d[cls.KEY_UNASSIGNED_READS]
            ),
        )

    def __init__(
        self,
        index: int,
        number_of_columns: int,
        all_bipart_stats: Iterable[BiPartStats] | None = None,
        unassigned_reads: Iterable[UnassignedReadStats] | None = None,
    ) -> None:
        """Initialize."""
        self.__index: int = index
        self.__number_of_columns: int = number_of_columns
        self.__bipart_stats: list[BiPartStats] = (
            list(all_bipart_stats) if all_bipart_stats is not None else []
        )
        self.__unassigned_reads: list[UnassignedReadStats] = (
            list(unassigned_reads) if unassigned_reads is not None else []
        )

    def index(self) -> int:
        """Return the index.

        Returns
        -------
        int
            Index

        """
        return self.__index

    def number_of_columns(self) -> int:
        """Return the number of columns.

        Returns
        -------
        int
            Number of columns

        """
        return self.__number_of_columns

    def iter_biparts(self) -> Iterator[BiPartStats]:
        """Return an iterator over the strip stats.

        Yields
        ------
        BiPartStats
            BiPartStats

        """
        return iter(self.__bipart_stats)

    def get_bipart(self, idx: int) -> BiPartStats:
        """Get a bipart stats.

        Parameters
        ----------
        idx : int
            Index of the bipart stats

        Returns
        -------
        BiPartStats
            BiPartStats at the corresponding index

        """
        return self.__bipart_stats[idx]

    def number_of_biparts(self) -> int:
        """Return the number of bipart stats.

        Returns
        -------
        int
            Number of bipart stats

        """
        return len(self.__bipart_stats)

    def iter_unassigned_reads(self) -> Iterator[UnassignedReadStats]:
        """Return an iterator over the unassigned reads.

        Yields
        ------
        UnassignedReadStats
            UnassignedReadStats

        """
        return iter(self.__unassigned_reads)

    def get_unassigned_read(self, idx: int) -> UnassignedReadStats:
        """Get an unassigned read.

        Parameters
        ----------
        idx : int
            Index of the unassigned read

        Returns
        -------
        UnassignedReadStats
            UnassignedReadStats at the corresponding index

        """
        return self.__unassigned_reads[idx]

    def number_of_unassigned_reads(self) -> int:
        """Return the number of unassigned reads.

        Returns
        -------
        int
            Number of unassigned reads

        """
        return len(self.__unassigned_reads)

    def set_number_of_columns(self, number_of_columns: int) -> None:
        """Set the number of columns.

        Parameters
        ----------
        number_of_columns : int
            Number of columns

        """
        self.__number_of_columns = number_of_columns

    def add_bipart_stats(self, bipart_stats: BiPartStats) -> None:
        """Add a bipart stats.

        Parameters
        ----------
        bipart_stats : BiPartStats
            BiPartStats

        """
        self.__bipart_stats.append(bipart_stats)

    def add_unassigned_read(self, unassigned_read: UnassignedReadStats) -> None:
        """Add unassigned read.

        Parameters
        ----------
        unassigned_read : UnassignedReadStats
            UnassignedReadStats

        """
        self.__unassigned_reads.append(unassigned_read)

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_INDEX: self.__index,
            self.KEY_NUMBER_OF_COLUMNS: self.__number_of_columns,
            self.KEY_BIPART_STATS: [
                bipart_stats.to_dict() for bipart_stats in self.__bipart_stats
            ],
            self.KEY_UNASSIGNED_READS: [
                unassigned_read_stats.to_dict()
                for unassigned_read_stats in self.__unassigned_reads
            ],
        }


class UnassignedReadStats:
    """Manually added read in a strip class."""

    KEY_READ_ID: str = "read_id"
    KEY_NUMBER_OF_ONES: str = "number_of_ones"
    KEY_NUMBER_OF_ZEROS: str = "number_of_zeros"

    @classmethod
    def from_dict(cls, d: dict) -> UnassignedReadStats:
        """Convert dict to UnassignedReadStats.

        Parameters
        ----------
        d : dict
            Dictionnary of view

        Returns
        -------
        UnassignedReadStats
            UnassignedReadStats

        """
        return cls(
            d[cls.KEY_READ_ID],
            d[cls.KEY_NUMBER_OF_ONES],
            d[cls.KEY_NUMBER_OF_ZEROS],
        )

    def __init__(
        self,
        read_id: str,
        number_of_ones: int,
        numbers_of_zeros: int,
    ) -> None:
        """Initialize."""
        self.__read_id: str = read_id
        self.__number_of_ones: int = number_of_ones
        self.__numbers_of_zeros: int = numbers_of_zeros

    def read_id(self) -> str:
        """Return the read id.

        Returns
        -------
        str
            Read id

        """
        return self.__read_id

    def number_of_ones(self) -> int:
        """Return the number of ones.

        Returns
        -------
        int
            Number of ones

        """
        return self.__number_of_ones

    def number_of_zeros(self) -> int:
        """Return the number of zeros.

        Returns
        -------
        int
            Number of zeros

        """
        return self.__numbers_of_zeros

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_READ_ID: self.__read_id,
            self.KEY_NUMBER_OF_ONES: self.__number_of_ones,
            self.KEY_NUMBER_OF_ZEROS: self.__numbers_of_zeros,
        }


class QBCTrashStripStats:
    """QBC strip trash class."""

    KEY_INDEX: str = "index"
    KEY_MATRIX_STATS: str = "matrix_stats"

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> QBCTrashStripStats:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dict(yaml.safe_load(file))

    @classmethod
    def from_dict(cls, d: dict) -> QBCTrashStripStats:
        """Convert dict to StripTrash.

        Parameters
        ----------
        d : dict
            Dictionnary of view

        Returns
        -------
        StripTrash
            StripTrash

        """
        return cls(
            d[cls.KEY_INDEX],
            BinMatrixStats.from_dict(d[cls.KEY_MATRIX_STATS]),
        )

    def __init__(
        self,
        index: int,
        matrix_stats: BinMatrixStats,
    ) -> None:
        """Initialize."""
        self.__index: int = index
        self.__matrix_stats: BinMatrixStats = matrix_stats

    def index(self) -> int:
        """Return the index.

        Returns
        -------
        int
            Index

        """
        return self.__index

    def matrix_stats(self) -> BinMatrixStats:
        """Return the matrix stats.

        Returns
        -------
        BinMatrixStats
            BinMatrixStats

        """
        return self.__matrix_stats

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_INDEX: self.__index,
            self.KEY_MATRIX_STATS: self.__matrix_stats.to_dict(),
        }

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with yaml_path.open("w") as file:
            yaml.dump(self.to_dict(), file, Dumper=Dumper, sort_keys=False)


class QBCStripStatsCollection:
    """QBC strip stats collection."""

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> QBCStripStatsCollection:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dicts(yaml.safe_load(file))

    @classmethod
    def from_dicts(
        cls,
        strips_stats: list[dict],
    ) -> QBCStripStatsCollection:
        """Create an empty strip stats collection."""
        return QBCStripStatsCollection(
            QBCStripStats.from_dict(strip_stats) for strip_stats in strips_stats
        )

    def __init__(self, strip_stats: Iterable[QBCStripStats] | None = None) -> None:
        """Initialize."""
        self.__strip_stats: list[QBCStripStats] = (
            list(strip_stats) if strip_stats is not None else []
        )

    def append(self, strip_stats: QBCStripStats) -> None:
        """Append a strip stats.

        Parameters
        ----------
        strip_stats : QBCStripStats
            QBCStripStats

        """
        self.__strip_stats.append(strip_stats)

    def __getitem__(self, idx: int) -> QBCStripStats:
        """Get a strip stats.

        Parameters
        ----------
        idx : int
            Index of the strip stats

        Returns
        -------
        QBCStripStats
            QBCStripStats at the corresponding index

        """
        return self.__strip_stats[idx]

    def __iter__(self) -> Iterator[QBCStripStats]:
        """Return an iterator over the strip stats.

        Yields
        ------
        QBCStripStats
            QBCStripStats

        """
        return iter(self.__strip_stats)

    def __len__(self) -> int:
        """Return the length.

        Returns
        -------
        int
            Length

        """
        return len(self.__strip_stats)

    def to_dicts(self) -> list[dict]:
        """Convert to list.

        Returns
        -------
        list
            List

        """
        return [strip_stats.to_dict() for strip_stats in self]

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with yaml_path.open("w") as file:
            yaml.dump(self.to_dicts(), file, Dumper=Dumper, sort_keys=False)


class QBCProcessStats:
    """QBC process stats."""

    KEY_HIGH_QUALITY_STRIPS_STATS = "high_quality_strips_stats"
    KEY_TRASH_STRIP_STATS = "trash_strip_stats"

    @classmethod
    def from_dict(cls, d: dict) -> QBCProcessStats:
        """Convert dict to QBCProcessStats.

        Parameters
        ----------
        d : dict
            Dictionnary of view

        Returns
        -------
        QBCProcessStats
            QBCProcessStats

        """
        return cls(
            QBCStripStatsCollection.from_dicts(d[cls.KEY_HIGH_QUALITY_STRIPS_STATS]),
            QBCTrashStripStats.from_dict(d[cls.KEY_TRASH_STRIP_STATS])
            if d[cls.KEY_TRASH_STRIP_STATS] is not None
            else None,
        )

    def __init__(
        self,
        high_quality_strips_stats: QBCStripStatsCollection,
        trash_strip_stats: QBCTrashStripStats | None,
    ) -> None:
        """Initialize."""
        self.__high_quality_strips_stats: QBCStripStatsCollection = (
            high_quality_strips_stats
        )
        self.__trash_strip_stats: QBCTrashStripStats | None = trash_strip_stats

    def high_quality_strips_stats(self) -> QBCStripStatsCollection:
        """Return the high quality strips stats.

        Returns
        -------
        QBCStripStatsCollection
            QBCStripStatsCollection

        """
        return self.__high_quality_strips_stats

    def trash_strip_stats(self) -> QBCTrashStripStats | None:
        """Return the trash strip stats.

        Returns
        -------
        QBCTrashStripStats
            QBCTrashStripStats

        """
        return self.__trash_strip_stats

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_HIGH_QUALITY_STRIPS_STATS: self.__high_quality_strips_stats.to_dicts(),
            self.KEY_TRASH_STRIP_STATS: (
                self.__trash_strip_stats.to_dict()
                if self.__trash_strip_stats is not None
                else None
            ),
        }
