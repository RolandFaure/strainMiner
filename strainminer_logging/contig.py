"""Contig logging module."""

from __future__ import annotations

from typing import TYPE_CHECKING, Iterable, Iterator

from strainminer_logging.strips.hca import HCAProcessStats

if TYPE_CHECKING:
    from pathlib import Path

import yaml  # type: ignore[import-untyped]

try:
    from yaml import CDumper as Dumper
except ImportError:
    from yaml import Dumper

import strainminer_logging.loci_window as lociwin_log


class ContigStats:
    """Contig stats."""

    KEY_CONTIG_NAME = "contig_name"
    KEY_CONTIG_LENGTH = "contig_length"

    @classmethod
    def from_dict(cls, d: dict) -> ContigStats:
        """Create from dict."""
        return cls(
            contig_name=d[cls.KEY_CONTIG_NAME],
            contig_length=d[cls.KEY_CONTIG_LENGTH],
        )

    def __init__(
        self,
        contig_name: str,
        contig_length: int,
    ) -> None:
        """Initialize."""
        self.__contig_name = contig_name
        self.__contig_length = contig_length

    def contig_name(self) -> str:
        """Get contig name."""
        return self.__contig_name

    def contig_length(self) -> int:
        """Get contig length."""
        return self.__contig_length

    def to_dict(self) -> dict:
        """Return dict."""
        return {
            self.KEY_CONTIG_NAME: self.__contig_name,
            self.KEY_CONTIG_LENGTH: self.__contig_length,
        }


class ContigProcessStats:
    """Contig process stats."""

    KEY_CONTIG_STATS = "contig_stats"
    KEY_LOCI_WINDOWS_STATS = "loci_windows_stats"

    @classmethod
    def from_dict(cls, d: dict) -> ContigProcessStats:
        """Create from dict."""
        return cls(
            contig_stats=ContigStats.from_dict(d[cls.KEY_CONTIG_STATS]),
            loci_windows_stats=lociwin_log.LociWindowStatsCollection.from_dicts(
                d[cls.KEY_LOCI_WINDOWS_STATS]
            ),
        )

    def __init__(
        self,
        contig_stats: ContigStats,
        loci_windows_stats: lociwin_log.LociWindowStatsCollection,
    ) -> None:
        """Initialize."""
        self.__contig_stats = contig_stats
        self.__loci_windows_stats = loci_windows_stats

    def contig_stats(self) -> ContigStats:
        """Get contig stats."""
        return self.__contig_stats

    def loci_windows_stats(self) -> lociwin_log.LociWindowStatsCollection:
        """Get loci windows stats."""
        return self.__loci_windows_stats

    def to_dict(self) -> dict:
        """Return dict."""
        return {
            self.KEY_CONTIG_STATS: self.__contig_stats.to_dict(),
            self.KEY_LOCI_WINDOWS_STATS: self.__loci_windows_stats.to_dicts(),
        }


class ContigProcessStatsCollection(list[ContigProcessStats]):
    """Contig process stats collection."""

    @classmethod
    def from_yaml(cls, yaml_path: Path) -> ContigProcessStatsCollection:
        """Create from YAML file."""
        with open(yaml_path, "r") as f:
            return cls.from_dicts(yaml.safe_load(f))

    @classmethod
    def from_dicts(cls, dicts: Iterable[dict]) -> ContigProcessStatsCollection:
        """Create from dicts."""
        return cls(ContigProcessStats.from_dict(d) for d in dicts)

    def to_dicts(self) -> list[dict]:
        """Return the dicts."""
        return [stats.to_dict() for stats in self]

    def to_yaml(self, yaml_path: Path) -> None:
        """Write to YAML file."""
        with open(yaml_path, "w") as f:
            yaml.dump(self.to_dicts(), f, Dumper=Dumper, sort_keys=False)
