"""Loci windows views."""

from __future__ import annotations

from typing import Iterable, Iterator

import strainminer_logging.postprocess as postprocess_log
import strainminer_logging.strips.hca as hca_log
import strainminer_logging.strips.qbc as qbc_log


class LociWindowStats:
    """Core loci windows stats.

    It corresponds to LociWindowStats but without the contig name.
    """

    KEY_START_POS = "start_pos"
    KEY_STOP_POS = "stop_pos"

    @classmethod
    def from_dict(cls, dict_: dict) -> LociWindowStats:
        """Create from dict."""
        return cls(
            dict_[cls.KEY_START_POS],
            dict_[cls.KEY_STOP_POS],
        )

    @classmethod
    def from_loci_window_stats(
        cls,
        loci_window_stats: LociWindowStats,
    ) -> LociWindowStats:
        """Create from LociWindowStats."""
        return cls(
            loci_window_stats.start_pos(),
            loci_window_stats.stop_pos(),
        )

    def __init__(
        self,
        start_pos: int,
        stop_pos: int,
    ) -> None:
        """Initialize."""
        self.__start_pos = start_pos
        self.__stop_pos = stop_pos

    def start_pos(self) -> int:
        """Return the start position."""
        return self.__start_pos

    def stop_pos(self) -> int:
        """Return the stop position."""
        return self.__stop_pos

    def to_dict(self) -> dict:
        """Return the dict."""
        return {
            self.KEY_START_POS: self.__start_pos,
            self.KEY_STOP_POS: self.__stop_pos,
        }


class LociWindowProcessStats:
    """Loci window process stats."""

    KEY_LOCI_WINDOW_STATS = "loci_window_stats"
    KEY_HCA_PROCESS_STATS = "hca_process_stats"
    KEY_QBC_PROCESS_STATS = "qbc_process_stats"
    KEY_POSTPROCESS_STATS = "postprocess_stats"

    @classmethod
    def from_dict(cls, d: dict) -> LociWindowProcessStats:
        """Create from dict."""
        return cls(
            LociWindowStats.from_dict(d[cls.KEY_LOCI_WINDOW_STATS]),
            hca_log.HCAProcessStats.from_dict(d[cls.KEY_HCA_PROCESS_STATS])
            if d[cls.KEY_HCA_PROCESS_STATS] is not None
            else None,
            qbc_log.QBCProcessStats.from_dict(d[cls.KEY_QBC_PROCESS_STATS])
            if d[cls.KEY_QBC_PROCESS_STATS] is not None
            else None,
            postprocess_log.PostProcessStats.from_dict(d[cls.KEY_POSTPROCESS_STATS])
            if d[cls.KEY_POSTPROCESS_STATS] is not None
            else None,
        )

    def __init__(
        self,
        loci_window_stats: LociWindowStats,
        hca_process_stats: hca_log.HCAProcessStats | None,
        qbc_process_stats: qbc_log.QBCProcessStats | None,
        postprocess_stats: postprocess_log.PostProcessStats | None,
    ) -> None:
        """Initialize."""
        self.__loci_window_stats = loci_window_stats
        self.__hca_process_stats = hca_process_stats
        self.__qbc_process_stats = qbc_process_stats
        self.__postprocess_stats = postprocess_stats

    def loci_window_stats(self) -> LociWindowStats:
        """Return loci window stats."""
        return self.__loci_window_stats

    def hca_process_stats(self) -> hca_log.HCAProcessStats | None:
        """Return HCA process stats."""
        return self.__hca_process_stats

    def qbc_process_stats(self) -> qbc_log.QBCProcessStats | None:
        """Return QBC process stats."""
        return self.__qbc_process_stats

    def postprocess_stats(self) -> postprocess_log.PostProcessStats | None:
        """Return postprocess stats."""
        return self.__postprocess_stats

    def to_dict(self) -> dict:
        """Return the dict."""
        return {
            self.KEY_LOCI_WINDOW_STATS: self.__loci_window_stats.to_dict(),
            self.KEY_HCA_PROCESS_STATS: self.__hca_process_stats.to_dict()
            if self.__hca_process_stats is not None
            else None,
            self.KEY_QBC_PROCESS_STATS: self.__qbc_process_stats.to_dict()
            if self.__qbc_process_stats is not None
            else None,
            self.KEY_POSTPROCESS_STATS: self.__postprocess_stats.to_dict()
            if self.__postprocess_stats is not None
            else None,
        }


class LociWindowStatsCollection:
    """Loci window stats collection."""

    @classmethod
    def from_dicts(cls, dicts: Iterable[dict]) -> LociWindowStatsCollection:
        """Create from dicts."""
        return cls(LociWindowProcessStats.from_dict(d) for d in dicts)

    def __init__(
        self,
        loci_window_process_stats_collection: Iterable[LociWindowProcessStats]
        | None = None,
    ) -> None:
        """Initialize."""
        self.__all_loci_window_process_stats = (
            list(loci_window_process_stats_collection)
            if loci_window_process_stats_collection is not None
            else []
        )

    def append(self, loci_window_process_stats: LociWindowProcessStats) -> None:
        """Append."""
        self.__all_loci_window_process_stats.append(loci_window_process_stats)

    def extend(
        self,
        loci_window_process_stats_collection: Iterable[LociWindowProcessStats],
    ) -> None:
        """Extend."""
        self.__all_loci_window_process_stats.extend(
            loci_window_process_stats_collection
        )

    def __len__(self) -> int:
        """Return the length."""
        return len(self.__all_loci_window_process_stats)

    def __iter__(self) -> Iterator[LociWindowProcessStats]:
        """Return an iterator over the loci window process stats."""
        return iter(self.__all_loci_window_process_stats)

    def __getitem__(self, index: int) -> LociWindowProcessStats:
        """Return the loci window process stats at the given index."""
        return self.__all_loci_window_process_stats[index]

    def to_dicts(self) -> list[dict]:
        """Return the dicts."""
        return [stats.to_dict() for stats in self.__all_loci_window_process_stats]
