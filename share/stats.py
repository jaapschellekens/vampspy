"""
stats.py — Statistical functions for VAMPS Python scripting.

Port of stats.sl.  All functions operate on lists of lists (matrices),
with data organised column-wise.  Column 0 is treated as x-data for
bivariate functions.

If numpy is available the functions delegate to it for speed; otherwise
a pure-Python fallback is used.

Functions
---------
median(mtx)   -> list          median for each column
mean(mtx)     -> list          mean for each column
rmsq(mtx)     -> list          root mean square for each column
mmax(mtx)     -> list          maximum for each column
mmin(mtx)     -> list          minimum for each column
adev(mtx)     -> list          absolute mean deviation for each column
sdev(mtx)     -> list          standard deviation for each column
linreg(mtx)   -> [[A], [B]]    linear regression coefficients
corr(mtx)     -> list          correlation coefficient vs. column 0
covar(mtx)    -> list          covariance vs. column 0
"""

import math


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _col(mtx, j):
    return [row[j] for row in mtx]


def _ncols(mtx):
    return len(mtx[0]) if mtx else 0


def _nrows(mtx):
    return len(mtx)


# ---------------------------------------------------------------------------
# Univariate
# ---------------------------------------------------------------------------

def median(mtx):
    """Median for each column."""
    m, n = _nrows(mtx), _ncols(mtx)
    result = []
    for j in range(n):
        col = sorted(_col(mtx, j))
        mid = m // 2
        result.append(col[mid] if m & 1 else (col[mid - 1] + col[mid]) / 2.0)
    return result


def mean(mtx):
    """Mean for each column."""
    m, n = _nrows(mtx), _ncols(mtx)
    return [sum(_col(mtx, j)) / m for j in range(n)]


def rmsq(mtx):
    """Root mean square for each column."""
    m, n = _nrows(mtx), _ncols(mtx)
    return [math.sqrt(sum(v * v for v in _col(mtx, j)) / m) for j in range(n)]


def mmax(mtx):
    """Maximum for each column."""
    return [max(_col(mtx, j)) for j in range(_ncols(mtx))]


def mmin(mtx):
    """Minimum for each column."""
    return [min(_col(mtx, j)) for j in range(_ncols(mtx))]


def adev(mtx):
    """Absolute mean deviation for each column."""
    m, n = _nrows(mtx), _ncols(mtx)
    result = []
    for j in range(n):
        col = _col(mtx, j)
        avg = sum(col) / m
        result.append(sum(abs(v - avg) for v in col) / m)
    return result


def sdev(mtx):
    """Standard deviation for each column (population, divide by N)."""
    m, n = _nrows(mtx), _ncols(mtx)
    result = []
    for j in range(n):
        col  = _col(mtx, j)
        avg  = sum(col) / m
        diffs = [v - avg for v in col]
        sum2 = sum(d * d for d in diffs)
        ssum = sum(diffs)
        var  = (sum2 - ssum * ssum / m) / m
        result.append(math.sqrt(max(var, 0.0)))
    return result


# ---------------------------------------------------------------------------
# Bivariate (x = column 0, y = columns 1…n-1)
# ---------------------------------------------------------------------------

def linreg(mtx):
    """Linear regression: y = A*x + B for each y-column vs column 0.

    Returns [[A0, A1, ...], [B0, B1, ...]] where A0=1, B0=0 by convention.
    """
    m, n = _nrows(mtx), _ncols(mtx)
    x_col = _col(mtx, 0)
    sumx  = sum(x_col)
    sumx2 = sum(v * v for v in x_col)
    frx   = m * sumx2 - sumx * sumx

    A = [0.0] * n
    B = [0.0] * n
    A[0] = 1.0

    for j in range(1, n):
        y_col = _col(mtx, j)
        sumy  = sum(y_col)
        sumxy = sum(x_col[i] * y_col[i] for i in range(m))
        A[j] = (m * sumxy - sumx * sumy) / frx
        B[j] = (sumy * sumx2 - sumx * sumxy) / frx

    return [A, B]


def corr(mtx):
    """Pearson correlation coefficient of each column vs column 0."""
    m, n = _nrows(mtx), _ncols(mtx)
    x_col = _col(mtx, 0)
    sumx  = sum(x_col)
    sumx2 = sum(v * v for v in x_col)
    frx   = m * sumx2 - sumx * sumx

    result = [1.0]
    for j in range(1, n):
        y_col = _col(mtx, j)
        sumy  = sum(y_col)
        sumy2 = sum(v * v for v in y_col)
        sumxy = sum(x_col[i] * y_col[i] for i in range(m))
        fry   = m * sumy2 - sumy * sumy
        denom = math.sqrt(frx * fry)
        result.append((m * sumxy - sumx * sumy) / denom if denom else 0.0)
    return result


def covar(mtx):
    """Covariance of each column vs column 0."""
    m, n = _nrows(mtx), _ncols(mtx)
    mu = mean(mtx)
    result = []
    for j in range(n):
        dxy = sum(
            (mtx[i][0] - mu[0]) * (mtx[i][j] - mu[j])
            for i in range(m)
        )
        result.append(dxy / m)
    return result
