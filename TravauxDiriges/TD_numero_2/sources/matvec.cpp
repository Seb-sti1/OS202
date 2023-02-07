// Produit matrice-vecteur
#include <cassert>
#include <vector>
#include <iostream>
#include <mpi.h>

// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix(int dim);
    Matrix(int nrows, int ncols);
    Matrix(const Matrix &A) = delete;
    Matrix(Matrix &&A) = default;
    ~Matrix() = default;

    Matrix &operator=(const Matrix &A) = delete;
    Matrix &operator=(Matrix &&A) = default;

    double &operator()(int i, int j)
    {
        return m_arr_coefs[i + j * m_nrows];
    }
    double operator()(int i, int j) const
    {
        return m_arr_coefs[i + j * m_nrows];
    }

    std::vector<double> operator*(const std::vector<double> &u) const;

    std::ostream &print(std::ostream &out) const
    {
        const Matrix &A = *this;
        out << "[\n";
        for (int i = 0; i < m_nrows; ++i)
        {
            out << " [ ";
            for (int j = 0; j < m_ncols; ++j)
            {
                out << A(i, j) << " ";
            }
            out << " ]\n";
        }
        out << "]";
        return out;
    }

private:
    int m_nrows, m_ncols;
    std::vector<double> m_arr_coefs;
};
// ---------------------------------------------------------------------
inline std::ostream &
operator<<(std::ostream &out, const Matrix &A)
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream &
operator<<(std::ostream &out, const std::vector<double> &u)
{
    out << "[ ";
    for (const auto &x : u)
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double> Matrix::operator*(const std::vector<double> &u) const
{
    const Matrix &A = *this;
    assert(u.size() == unsigned(m_ncols));
    std::vector<double> v(m_nrows, 0.);
    for (int i = 0; i < m_nrows; ++i)
    {
        for (int j = 0; j < m_ncols; ++j)
        {
            v[i] += A(i, j) * u[j];
        }
    }
    return v;
}

// =====================================================================
Matrix::Matrix(int dim) : m_nrows(dim), m_ncols(dim),
                          m_arr_coefs(dim * dim)
{
    for (int i = 0; i < dim; ++i)
    {
        for (int j = 0; j < dim; ++j)
        {
            (*this)(i, j) = (i + j) % dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix(int nrows, int ncols) : m_nrows(nrows), m_ncols(ncols),
                                       m_arr_coefs(nrows * ncols)
{
    int dim = (nrows > ncols ? nrows : ncols);
    for (int i = 0; i < nrows; ++i)
    {
        for (int j = 0; j < ncols; ++j)
        {
            (*this)(i, j) = (i + j) % dim;
        }
    }
}

bool sameVector(std::vector<double> v_save, std::vector<double> v, int N)
{
    for (int i = 0; i < N; i++)
    {
        if (v[i] != v_save[i])
        {
            return false;
        }
    }
    return true;
}

// =====================================================================
int main(int nargs, char *argv[])
{

    MPI_Comm global;
    int rank, nbp;
    MPI_Init(&nargs, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &global);
    MPI_Comm_size(global, &nbp);
    MPI_Comm_rank(global, &rank);

    if (rank == 0)
        std::cout << std::endl;

    const int N = 120;
    Matrix A(N);
    std::vector<double> u(N);
    for (int i = 0; i < N; ++i)
        u[i] = i + 1;
    std::vector<double> v_save = A * u;
    std::vector<double> v;

    // calculation without mpi
    if (rank == 0)
    {
        std::cout << "A.u = " << v_save << std::endl;

        v.resize(N, 0.);
    }

    assert(u.size() == unsigned(N));
    assert(N % nbp == 0);

    int Nloc = N / nbp;
    int offset = rank * Nloc;

    MPI_Barrier(global);

    // compute cols offset to offset + Nloc
    std::vector<double> vloc(N, 0.);
    for (int i = 0; i < N; ++i)
    {
        for (int j = offset; j < offset + Nloc; ++j)
        {
            vloc[i] += A(i, j) * u[j];
        }
    }

    MPI_Reduce(vloc.data(), v.data(), N, MPI_DOUBLE,
               MPI_SUM, 0, global);

    if (rank == 0)
    {
        std::cout << std::endl;
        std::cout << "By cols" << std::endl;
        if (sameVector(v_save, v, N))
        {
            std::cout << "Valid" << std::endl;
        }
        else
        {
            std::cout << "Invalid" << std::endl;
            std::cout << v << std::endl;
        }

        v.resize(N);
    }

    MPI_Barrier(global);

    vloc.resize(Nloc);
    for (int i = 0; i < Nloc; ++i)
        vloc[i] = 0;

    for (int i = 0; i < Nloc; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            vloc[i] += A(i + offset, j) * u[j];
        }
    }

    MPI_Gather(vloc.data(), Nloc, MPI_DOUBLE, v.data(), Nloc, MPI_DOUBLE, 0, global);

    if (rank == 0)
    {
        std::cout << std::endl;
        std::cout << "By rows" << std::endl;
        if (sameVector(v_save, v, N))
        {
            std::cout << "Valid" << std::endl;
        }
        else
        {
            std::cout << "Invalid" << std::endl;
            std::cout << v << std::endl;
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
