"""Binary matrix views."""

from __future__ import annotations

from typing import Any

import pandas as pd


class BinMatrixStats:
    """Binary matrix stats."""

    __NUMBER_OF_ROWS_KEY: str = "number_of_rows"
    __NUMBER_OF_COLUMNS_KEY: str = "number_of_columns"
    __NUMBER_OF_ONES_KEY: str = "number_of_ones"
    __NUMBER_OF_ZEROS_KEY: str = "number_of_zeros"
    __DENSITY_KEY: str = "density"
    __SPARSITY_KEY: str = "sparsity"

    @classmethod
    def from_dict(cls, bin_matrix_stats_dict: dict[str, Any]) -> BinMatrixStats:
        """Instantiate binary matrix stats from a dictionnary.

        Parameters
        ----------
        bin_matrix_stats_dict : dict
            Binary matrix stats dictionnary

        Returns
        -------
        BinMatrixStats
            Binary matrix stats

        """
        return BinMatrixStats(
            bin_matrix_stats_dict[cls.__NUMBER_OF_ROWS_KEY],
            bin_matrix_stats_dict[cls.__NUMBER_OF_COLUMNS_KEY],
            bin_matrix_stats_dict[cls.__NUMBER_OF_ONES_KEY],
            bin_matrix_stats_dict[cls.__NUMBER_OF_ZEROS_KEY],
            bin_matrix_stats_dict[cls.__DENSITY_KEY],
            bin_matrix_stats_dict[cls.__SPARSITY_KEY],
        )

    @classmethod
    def from_bin_dataframe(cls, df: pd.DataFrame) -> BinMatrixStats:
        """Create from bin dataframe."""
        number_of_ones = (df == 1).sum().sum()
        number_of_zeros = (df == 0).sum().sum()
        number_of_cells = df.shape[0] * df.shape[1]
        return cls(
            df.shape[0],
            df.shape[1],
            number_of_ones,
            number_of_zeros,
            number_of_ones / number_of_cells,
            number_of_zeros / number_of_cells,
        )

    def __init__(  # noqa: PLR0913
        self,
        number_of_rows: int,
        number_of_columns: int,
        number_of_ones: int,
        number_of_zeros: int,
        density: float,
        sparsity: float,
    ) -> None:
        """Initialize binary matrix stats."""
        self.__number_of_rows = number_of_rows
        self.__number_of_columns = number_of_columns
        self.__number_of_ones = number_of_ones
        self.__number_of_zeros = number_of_zeros
        self.__density = density
        self.__sparsity = sparsity

    def number_of_rows(self) -> int:
        """Give the number of rows.

        Returns
        -------
        int
            Number of rows

        """
        return self.__number_of_rows

    def number_of_columns(self) -> int:
        """Give the number of columns.

        Returns
        -------
        int
            Number of columns

        """
        return self.__number_of_columns

    def number_of_ones(self) -> int:
        """Give the number of ones.

        Returns
        -------
        int
            Number of ones

        """
        return self.__number_of_ones

    def number_of_zeros(self) -> int:
        """Give the number of zeros.

        Returns
        -------
        int
            Number of zeros

        """
        return self.__number_of_zeros

    def density(self) -> float:
        """Density.

        Returns
        -------
        float
            Density

        """
        return self.__density

    def sparsity(self) -> float:
        """Sparsity.

        Returns
        -------
        float
            Sparsity

        """
        return self.__sparsity

    def to_dict(self) -> dict:
        """Convert binary matrix stats to dict.

        Returns
        -------
        dict
            Dictionnary

        """
        return {
            self.__NUMBER_OF_ROWS_KEY: self.__number_of_rows,
            self.__NUMBER_OF_COLUMNS_KEY: self.__number_of_columns,
            self.__NUMBER_OF_ONES_KEY: self.__number_of_ones,
            self.__NUMBER_OF_ZEROS_KEY: self.__number_of_zeros,
            self.__DENSITY_KEY: self.__density,
            self.__SPARSITY_KEY: self.__sparsity,
        }

    def __str__(self) -> str:
        """Print stats."""
        return "\n".join(
            [
                f"Dimensions: {self.number_of_rows()} x {self.number_of_columns()}"
                f" ({self.number_of_rows() * self.number_of_columns()})",
                f"Number of ones: {self.number_of_ones()}",
                f"Number of zeros: {self.number_of_zeros()}",
                f"Density: {self.density()}",
                f"Sparsity: {self.sparsity()}",
            ],
        )
