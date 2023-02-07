#include <cassert>
#include <vector>
#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <chrono>

inline std::ostream &
operator<<(std::ostream &out, const std::vector<int> &u)
{
    out << "[ ";
    for (const auto &x : u)
        out << x << " ";
    out << " ]";
    return out;
}

int main(int argc, char *argv[])
{
    MPI_Comm global;
    int rank, nbp;
    MPI_Init(&argc, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &global);
    MPI_Comm_size(global, &nbp);
    MPI_Comm_rank(global, &rank);

    if (rank == 0)
        std::cout << std::endl;

    int N = 120000000;
    assert(N % nbp == 0);

    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::vector<int> randomBucket;
    std::vector<int> buckets;
    std::vector<int> scatSizes; // A array which contains the size of received data for each processes (useful only for root)
    std::vector<int> displs;

    std::vector<int> sortedBucket(N);
    std::vector<int> localBucket(N);

    if (rank == 0)
    {

        // ===== Generate random numbers
        srand((unsigned)time(0));
        int max = (((rand() % 5000) + nbp) / nbp) * nbp;
        int step = max / nbp;

        randomBucket.resize(N);
        for (int i = 0; i < N; i++)
        {
            randomBucket[i] = (rand() % (max - 1)) + 1;
        }

        // start chrono
        start = std::chrono::system_clock::now();

        // ===== Calculate size of each buckets
        scatSizes.resize(nbp, 0);
        displs.resize(nbp);
        displs[0] = 0;

        for (int i = 0; i < N; i++)
        {
            for (int rank_n = 0; rank_n < nbp; rank_n++)
            {
                if (rank_n * step <= randomBucket[i] && randomBucket[i] < (rank_n + 1) * step)
                {
                    scatSizes[rank_n]++;
                    break;
                }
            }
        }

        // ===== Calculate of offset in the vector
        for (int i = 0; i < nbp - 1; i++)
        {
            displs[i + 1] = displs[i] + scatSizes[i];
        }

        // ===== Reorder according for the bucket
        buckets.resize(N);
        std::vector<int> idx(nbp, 0);

        for (int i = 0; i < N; i++)
        {
            for (int rank_n = 0; rank_n < nbp; rank_n++)
            {
                if (rank_n * step <= randomBucket[i] && randomBucket[i] < (rank_n + 1) * step)
                {
                    buckets[displs[rank_n] + idx[rank_n]] = randomBucket[i];
                    idx[rank_n]++;
                    break;
                }
            }
        }

        // std::cout << is_permutation(randomBucket.begin(), randomBucket.end(), buckets.begin()) << std::endl;
    }

    int localSize;
    MPI_Scatter(scatSizes.data(), 1, MPI_INT, &localSize, 1, MPI_INT, 0, global);

    localBucket.resize(localSize);
    MPI_Scatterv(buckets.data(), scatSizes.data(), displs.data(), MPI_INT,
                 localBucket.data(), localSize, MPI_INT, 0, global);

    // std::cout << rank << ": I have " << localSize << " = " << localBucket << std::endl;

    std::sort(localBucket.begin(), localBucket.end());

    MPI_Gatherv(localBucket.data(), localSize, MPI_INT,
                sortedBucket.data(), scatSizes.data(), displs.data(),
                MPI_INT, 0, global);

    if (rank == 0)
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> mpi_elapsed_seconds = end - start;

        // std::cout <<  sortedBucket << std::endl;
        start = std::chrono::system_clock::now();

        std::sort(randomBucket.begin(), randomBucket.end());

        end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;

        bool correct = true;
        for (int i = 0; i < N; i++)
        {
            if (sortedBucket[i] != randomBucket[i])
            {
                std::cout << i << ": " << sortedBucket[i]
                          << "!= " << randomBucket[i] << std::endl;

                correct = false;
                // break;
            }
        }

        if (correct)
        {
            std::cout << "Sort successfull. Took mpi " << mpi_elapsed_seconds.count()
                      << " against " << elapsed_seconds.count() << " without. "
                      << elapsed_seconds.count() / mpi_elapsed_seconds.count() << "" << std::endl;
        }
        else
        {
            std::cout << "Sort failed." << std::endl;
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
