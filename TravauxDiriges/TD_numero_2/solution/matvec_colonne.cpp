// Produit matrice-vecteur
# include <cassert>
# include <vector>
# include <iostream>
# include <fstream>
# include <mpi.h>

// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix (int dim);
    Matrix( int nrows, int ncols, int firstColumn );
    Matrix( const Matrix& A ) = delete;
    Matrix( Matrix&& A ) = default;
    ~Matrix() = default;

    Matrix& operator = ( const Matrix& A ) = delete;
    Matrix& operator = ( Matrix&& A ) = default;
    
    double& operator () ( int i, int j ) {
        return m_arr_coefs[i + j*m_nrows];
    }
    double  operator () ( int i, int j ) const {
        return m_arr_coefs[i + j*m_nrows];
    }
    
    std::vector<double> operator * ( const std::vector<double>& u ) const;
    
    std::ostream& print( std::ostream& out ) const
    {
        const Matrix& A = *this;
        out << "[\n";
        for ( int i = 0; i < m_nrows; ++i ) {
            out << " [ ";
            for ( int j = 0; j < m_ncols; ++j ) {
                out << A(i,j) << " ";
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
inline std::ostream& 
operator << ( std::ostream& out, const Matrix& A )
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream&
operator << ( std::ostream& out, const std::vector<double>& u )
{
    out << "[ ";
    for ( const auto& x : u )
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double> 
Matrix::operator * ( const std::vector<double>& u ) const
{
    const Matrix& A = *this;
    assert( u.size() == unsigned(m_ncols) );
    std::vector<double> v(m_nrows, 0.);
    for ( int i = 0; i < m_nrows; ++i ) {
        for ( int j = 0; j < m_ncols; ++j ) {
            v[i] += A(i,j)*u[j];
        }            
    }
    return v;
}

// =====================================================================
Matrix::Matrix (int dim) : m_nrows(dim), m_ncols(dim),
                           m_arr_coefs(dim*dim)
{
    for ( int i = 0; i < dim; ++ i ) {
        for ( int j = 0; j < dim; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix( int nrows, int ncols, int firstColumn ) 
    :   m_nrows(nrows), m_ncols(ncols),
        m_arr_coefs(nrows*ncols)
{
    int dim = (nrows > ncols ? nrows : ncols );
    for ( int i = 0; i < nrows; ++ i ) {
        for ( int j = 0; j < ncols; ++j ) {
            (*this)(i,j) = (i+j+firstColumn)%dim;
        }
    }    
}
// =====================================================================
int main( int nargs, char* argv[] )
{
    const int N = 120;
    MPI_Comm global;
    int rank, nbp;
    MPI_Init(&nargs, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &global);
    MPI_Comm_size(global, &nbp);
    MPI_Comm_rank(global, &rank);

    char bufferFileName[1024];
    sprintf(bufferFileName, "output%03d.txt", rank);
    std::ofstream out(bufferFileName);

    // Calcul du nombre de colonnes local par processus :
    int NLoc = N/nbp;
    // Calcul de l'indice globale de la première colonne de la matrice locale
    int firstCol = rank * NLoc;
    Matrix ALoc(N, NLoc, firstCol);
    out  << "ALoc : " << ALoc << std::endl;
    // Calcul du vecteur local u :
    std::vector<double> uLoc( NLoc );
    for ( int i = 0; i < NLoc; ++i ) uLoc[i] = i+firstCol+1;
    out << " uLoc : " << uLoc << std::endl;
    std::vector<double> vPart = ALoc*uLoc;
    out << "ALoc.uLoc = " << vPart << std::endl;
    std::vector<double> v(N);
    MPI_Allreduce(vPart.data(), v.data(), N, MPI_DOUBLE, MPI_SUM, global);
    out << "A.u = " << v << std::endl;

    out.close();
    MPI_Finalize();
    return EXIT_SUCCESS;
}
