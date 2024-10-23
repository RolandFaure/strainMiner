"""Postprocess logging module."""

from __future__ import annotations


class PostProcessStats:
    """Postprocess stats."""

    KEY_NUMBER_OF_INITIAL_HAPLOTYPES = "number_of_initial_haplotypes"
    KEY_NUMBER_OF_LOW_QUALITY_HAPLOTYPES = "number_of_low_quality_haplotypes"
    KEY_NUMBER_OF_FINAL_HAPLOTYPES = "number_of_final_haplotypes"

    @classmethod
    def from_dict(cls, d: dict) -> PostProcessStats:
        """Create from dictionary."""
        return PostProcessStats(
            number_of_initial_haplotypes=d[cls.KEY_NUMBER_OF_INITIAL_HAPLOTYPES],
            number_of_low_quality_haplotypes=d[
                cls.KEY_NUMBER_OF_LOW_QUALITY_HAPLOTYPES
            ],
            number_of_final_haplotypes=d[cls.KEY_NUMBER_OF_FINAL_HAPLOTYPES],
        )

    def __init__(
        self,
        number_of_initial_haplotypes: int,
        number_of_low_quality_haplotypes: int,
        number_of_final_haplotypes: int,
    ) -> None:
        """Initialize."""
        self.__number_of_initial_haplotypes = number_of_initial_haplotypes
        self.__number_of_low_quality_haplotypes = number_of_low_quality_haplotypes
        self.__number_of_final_haplotypes = number_of_final_haplotypes

    def number_of_initial_haplotypes(self) -> int:
        """Get number of initial haplotypes."""
        return self.__number_of_initial_haplotypes

    def number_of_low_quality_haplotypes(self) -> int:
        """Get number of low quality haplotypes."""
        return self.__number_of_low_quality_haplotypes

    def number_of_final_haplotypes(self) -> int:
        """Get number of final haplotypes."""
        return self.__number_of_final_haplotypes

    def to_dict(self) -> dict:
        """Return dict."""
        return {
            self.KEY_NUMBER_OF_INITIAL_HAPLOTYPES: self.__number_of_initial_haplotypes,
            self.KEY_NUMBER_OF_LOW_QUALITY_HAPLOTYPES: self.__number_of_low_quality_haplotypes,
            self.KEY_NUMBER_OF_FINAL_HAPLOTYPES: self.__number_of_final_haplotypes,
        }
