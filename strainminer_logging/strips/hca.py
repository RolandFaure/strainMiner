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


class HCAPreStripStatsCollection:
    """HCA bin matrix stats collection."""

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> HCAPreStripStatsCollection:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dicts(yaml.safe_load(file))

    @classmethod
    def from_dicts(cls, dicts: Iterable[dict]) -> HCAPreStripStatsCollection:
        """Create from dict."""
        return cls(
            (BinMatrixStats.from_dict(bin_matrix_stats) for bin_matrix_stats in dicts),
        )

    def __init__(
        self,
        bin_matrix_stats: Iterable[BinMatrixStats] | None = None,
    ) -> None:
        """Initialize."""
        self.__bin_matrix_stats: list[BinMatrixStats] = (
            list(bin_matrix_stats) if bin_matrix_stats is not None else []
        )

    def append(self, bin_matrix_stats: BinMatrixStats) -> None:
        """Append bin matrix stats."""
        self.__bin_matrix_stats.append(bin_matrix_stats)

    def extend(self, bin_matrix_stats: Iterable[BinMatrixStats]) -> None:
        """Extend bin matrix stats."""
        self.__bin_matrix_stats.extend(bin_matrix_stats)

    def __len__(self) -> int:
        """Return the length."""
        return len(self.__bin_matrix_stats)

    def __iter__(self) -> Iterator[BinMatrixStats]:
        """Return an iterator over the bin matrix stats."""
        return iter(self.__bin_matrix_stats)

    def __getitem__(self, index: int) -> BinMatrixStats:
        """Return the bin matrix stats at the given index."""
        return self.__bin_matrix_stats[index]

    def to_dicts(self) -> list[dict]:
        """Convert to dict."""
        return [bin_matrix_stats.to_dict() for bin_matrix_stats in self]

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with yaml_path.open("w") as file:
            yaml.dump(self.to_dicts(), file, Dumper=Dumper, sort_keys=False)


class LowQualityHCAStripStats:
    """HCA low quality strip stats."""

    KEY_STRIP_NUMBER = "strip_number"
    KEY_NUMBER_OF_COLUMNS = "number_of_columns"
    KEY_BIN_MATRICES_STATS = "bin_matrices_stats"

    @classmethod
    def from_dict(cls, dict_: dict) -> LowQualityHCAStripStats:
        """Create from dict."""
        return cls(
            strip_number=dict_[cls.KEY_STRIP_NUMBER],
            number_of_columns=dict_[cls.KEY_NUMBER_OF_COLUMNS],
            bin_matrices_stats=[
                BinMatrixStats.from_dict(bms_dict)
                for bms_dict in dict_[cls.KEY_BIN_MATRICES_STATS]
            ],
        )

    def __init__(
        self,
        strip_number: int,
        number_of_columns: int,
        bin_matrices_stats: Iterable[BinMatrixStats] | None = None,
    ) -> None:
        """Initialize."""
        self.__strip_number: int = strip_number
        self.__number_of_columns: int = number_of_columns
        self.__bin_matrices_stats: list[BinMatrixStats] = (
            list(bin_matrices_stats) if bin_matrices_stats is not None else []
        )

    def strip_number(self) -> int:
        """Return strip number."""
        return self.__strip_number

    def number_of_columns(self) -> int:
        """Return number of columns."""
        return self.__number_of_columns

    def bin_matrices_stats(self) -> list[BinMatrixStats]:
        """Return bin matrices stats."""
        return self.__bin_matrices_stats

    def to_dict(self) -> dict:
        """Return dict."""
        return {
            self.KEY_STRIP_NUMBER: self.strip_number(),
            self.KEY_NUMBER_OF_COLUMNS: self.number_of_columns(),
            self.KEY_BIN_MATRICES_STATS: [
                bms.to_dict() for bms in self.bin_matrices_stats()
            ],
        }


class LowQualityHCAStripStatsCollection:
    """HCA bin matrix stats collection."""

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> LowQualityHCAStripStatsCollection:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dicts(yaml.safe_load(file))

    @classmethod
    def from_dicts(cls, dicts: Iterable[dict]) -> LowQualityHCAStripStatsCollection:
        """Create from dict."""
        return cls(
            (
                LowQualityHCAStripStats.from_dict(bin_matrix_stats)
                for bin_matrix_stats in dicts
            ),
        )

    def __init__(
        self,
        bin_matrix_stats: Iterable[LowQualityHCAStripStats] | None = None,
    ) -> None:
        """Initialize."""
        self.__bin_matrix_stats: list[LowQualityHCAStripStats] = (
            list(bin_matrix_stats) if bin_matrix_stats is not None else []
        )

    def append(self, bin_matrix_stats: LowQualityHCAStripStats) -> None:
        """Append bin matrix stats."""
        self.__bin_matrix_stats.append(bin_matrix_stats)

    def extend(self, bin_matrix_stats: Iterable[LowQualityHCAStripStats]) -> None:
        """Extend bin matrix stats."""
        self.__bin_matrix_stats.extend(bin_matrix_stats)

    def __len__(self) -> int:
        """Return the length."""
        return len(self.__bin_matrix_stats)

    def __iter__(self) -> Iterator[LowQualityHCAStripStats]:
        """Return an iterator over the bin matrix stats."""
        return iter(self.__bin_matrix_stats)

    def __getitem__(self, index: int) -> LowQualityHCAStripStats:
        """Return the bin matrix stats at the given index."""
        return self.__bin_matrix_stats[index]

    def to_dicts(self) -> list[dict]:
        """Convert to dict."""
        return [bin_matrix_stats.to_dict() for bin_matrix_stats in self]

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with yaml_path.open("w") as file:
            yaml.dump(self.to_dicts(), file, Dumper=Dumper, sort_keys=False)


class HCAStripStats:
    """HCA strip views class."""

    KEY_INDEX: str = "index"
    KEY_NUMBER_OF_COLUMNS: str = "number_of_columns"
    KEY_BIPART_STATS: str = "bipart_stats"

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> HCAStripStats:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dict(yaml.safe_load(file))

    @classmethod
    def from_dict(cls, d: dict) -> HCAStripStats:
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
        )

    def __init__(
        self,
        index: int,
        number_of_columns: int,
        all_bipart_stats: Iterable[BiPartStats] | None = None,
    ) -> None:
        """Initialize."""
        self.__index: int = index
        self.__number_of_columns: int = number_of_columns
        self.__bipart_stats: list[BiPartStats] = (
            list(all_bipart_stats) if all_bipart_stats is not None else []
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

    def __iter__(self) -> Iterator[BiPartStats]:
        """Return an iterator over the strip stats.

        Yields
        ------
        BiPartStats
            BiPartStats

        """
        return iter(self.__bipart_stats)

    def __len__(self) -> int:
        """Return the length.

        Returns
        -------
        int
            Length

        """
        return len(self.__bipart_stats)

    def __getitem__(self, idx: int) -> BiPartStats:
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
            self.KEY_BIPART_STATS: [bipart_stats.to_dict() for bipart_stats in self],
        }

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with yaml_path.open("w") as file:
            yaml.dump(self.to_dict(), file, Dumper=Dumper, sort_keys=False)


class HCAStripStatsCollection:
    """HCA strip stats collection."""

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> HCAStripStatsCollection:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dicts(yaml.safe_load(file))

    @classmethod
    def from_dicts(
        cls,
        strips_stats: Iterable[dict],
    ) -> HCAStripStatsCollection:
        """Create an empty strip stats collection."""
        return HCAStripStatsCollection(
            HCAStripStats.from_dict(strip_stats) for strip_stats in strips_stats
        )

    def __init__(self, strip_stats: Iterable[HCAStripStats] | None = None) -> None:
        """Initialize."""
        self.__strip_stats: list[HCAStripStats] = (
            list(strip_stats) if strip_stats is not None else []
        )

    def append(self, strip_stats: HCAStripStats) -> None:
        """Append a strip stats.

        Parameters
        ----------
        strip_stats : HCAStripStats
            HCAStripStats

        """
        self.__strip_stats.append(strip_stats)

    def __getitem__(self, idx: int) -> HCAStripStats:
        """Get a strip stats.

        Parameters
        ----------
        idx : int
            Index of the strip stats

        Returns
        -------
        HCAStripStats
            HCAStripStats at the corresponding index

        """
        return self.__strip_stats[idx]

    def __iter__(self) -> Iterator[HCAStripStats]:
        """Return an iterator over the strip stats.

        Yields
        ------
        HCAStripStats
            HCAStripStats

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


class HCAProcessStats:
    """HCA process stats."""

    KEY_LOW_QUALITY_PRESTRIPS_STATS = "low_quality_prestrips_stats"
    KEY_HIGH_QUALITY_PRESTRIPS_STATS = "high_quality_prestrips_stats"
    KEY_LOW_QUALITY_HCA_STRIP_STATS = "low_quality_hca_strip_stats"
    KEY_HIGH_QUALITY_HCA_STRIP_STATS = "high_quality_hca_strip_stats"

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> HCAProcessStats:
        """Create from YAML file."""
        with yaml_path.open("r") as file:
            return cls.from_dict(yaml.safe_load(file))

    @classmethod
    def from_dict(
        cls,
        process_stats: dict,
    ) -> HCAProcessStats:
        """Create from dict."""
        return HCAProcessStats(
            HCAPreStripStatsCollection.from_dicts(
                process_stats[cls.KEY_LOW_QUALITY_PRESTRIPS_STATS]
            ),
            HCAPreStripStatsCollection.from_dicts(
                process_stats[cls.KEY_HIGH_QUALITY_PRESTRIPS_STATS]
            ),
            LowQualityHCAStripStatsCollection.from_dicts(
                process_stats[cls.KEY_LOW_QUALITY_HCA_STRIP_STATS]
            ),
            HCAStripStatsCollection.from_dicts(
                process_stats[cls.KEY_HIGH_QUALITY_HCA_STRIP_STATS]
            ),
        )

    def __init__(
        self,
        low_quality_prestrips_stats: HCAPreStripStatsCollection,
        high_quality_prestrips_stats: HCAPreStripStatsCollection,
        low_quality_hca_strip_stats: LowQualityHCAStripStatsCollection,
        high_quality_hca_strip_stats: HCAStripStatsCollection,
    ) -> None:
        """Initialize."""
        self.__low_quality_prestrips_stats = low_quality_prestrips_stats
        self.__high_quality_prestrips_stats = high_quality_prestrips_stats
        self.__low_quality_hca_strip_stats = low_quality_hca_strip_stats
        self.__high_quality_hca_strip_stats = high_quality_hca_strip_stats

    def low_quality_prestrips_stats(self) -> HCAPreStripStatsCollection:
        """Return low quality prestrips stats."""
        return self.__low_quality_prestrips_stats

    def high_quality_prestrips_stats(self) -> HCAPreStripStatsCollection:
        """Return high quality prestrips stats."""
        return self.__high_quality_prestrips_stats

    def low_quality_hca_strip_stats(self) -> LowQualityHCAStripStatsCollection:
        """Return low quality HCA strip stats."""
        return self.__low_quality_hca_strip_stats

    def high_quality_hca_strip_stats(self) -> HCAStripStatsCollection:
        """Return high quality HCA strip stats."""
        return self.__high_quality_hca_strip_stats

    def to_dict(self) -> dict:
        """Convert to dict.

        Returns
        -------
        dict
            Dictionary

        """
        return {
            self.KEY_LOW_QUALITY_PRESTRIPS_STATS: self.__low_quality_prestrips_stats.to_dicts(),
            self.KEY_HIGH_QUALITY_PRESTRIPS_STATS: self.__high_quality_prestrips_stats.to_dicts(),
            self.KEY_LOW_QUALITY_HCA_STRIP_STATS: self.__low_quality_hca_strip_stats.to_dicts(),
            self.KEY_HIGH_QUALITY_HCA_STRIP_STATS: self.__high_quality_hca_strip_stats.to_dicts(),
        }
