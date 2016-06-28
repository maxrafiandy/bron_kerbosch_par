/*
 * @author Max Rafiandy
 * Create Date: Jun, 28 2016
 * Version 1.0
 */

#include <omp.h>
#include <iostream>
#include <algorithm>
#include "bron_kerbosch.h"

Bron_kerbosch::Bron_kerbosch(std::vector<int> *graph, int n_vertex)
{
    this->graph = graph;
    this->print_screen = false;

}

Bron_kerbosch::Bron_kerbosch(std::vector<int> *graph, int n_vertex, bool print_screen)
{
    this->graph = graph;
    this->print_screen = print_screen;

}

int Bron_kerbosch::execute(std::vector<int> p)
{
    // open output file
    out_file.open("clique.txt",std::ios_base::out);

    bron_kerbosch(p);

    out_file.close();

    return EXIT_SUCCESS;
}

std::vector<int> Bron_kerbosch::neightbour(int vertex, std::vector<int> p)
{
    std::vector<int> n;

    // the graph should sorted in order to agile searching
    std::sort (graph[vertex].begin(), graph[vertex].end());

    int psize = p.size();
    // Load all linked vertex by p
    for(int i=0; i<psize; i++)
    {
        int v = p[i];
        // if vertex found in graph pust it back into n
        if (std::binary_search(graph[vertex].begin(), graph[vertex].end(), v))
        {
            n.push_back(v);
        }
    }
    return n;
}

void Bron_kerbosch::bron_kerbosch(std::vector<int> p)
{
    int psize = p.size();
    std::vector<int> r;


    // disable dynamic and set number threads to 2
    omp_set_dynamic(0);
    omp_set_nested(1);
    omp_set_num_threads(2);
    int thread_team = 1;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid == 0) thread_team = omp_get_num_threads();
        /*
         * for each vertex in p do:
         */
        #pragma omp barrier
        for(int i=tid; i<psize; i+=thread_team)
        {
            /*
             * since the last line for this loop,
             * so current vertex must be always in p[0]
             */
            std::vector<int> temp_p,temp_x;
            temp_p.assign(p.begin()+i,p.end());
            if (i) temp_x.assign(p.begin(),p.begin()+i);
            int vertex = temp_p[0];
            /*
             * init new r,p and x. Copy all r into new one
             * than append vertex into new_r
             */
            std::vector<int> new_r = r, new_p, new_x;
            new_r.push_back(vertex);

            omp_set_dynamic(0);
            omp_set_num_threads(2);
            #pragma omp parallel
            {
                #pragma omp single
                {
                    // new_p = P U N(vertex)
                    #pragma omp task firstprivate(vertex,temp_p)
                    new_p = neightbour(vertex,temp_p);

                    // new_x = X U N(vertex)
                    #pragma omp task firstprivate(vertex,temp_x)
                    new_x = neightbour(vertex,temp_x);

                    #pragma omp taskwait
                    bronkerbosch(new_r,new_p,new_x);
                }
            }
        }
    }
    return;
}

void Bron_kerbosch::bronkerbosch(std::vector<int> r, std::vector<int> p, std::vector<int> x)
{
    int psize = p.size();
    /*
     * if p and x are empty report r as
     * a maximal clique
     */
    if(!psize && !x.size())
    {
        #pragma omp critical
        {
            // print to file
            if (!print_screen)
            {
                out_file << "clique: [";
                int rsize = r.size();
                for(int i=0; i<rsize; i++)
                {
                    out_file << r[i];
                    if(i != rsize-1)
                    {
                        out_file << ",";
                    }
                }
                out_file << "]" << std::endl;
            }
            // print to screen
            else
            {
                std::cout << "clique: [";
                int rsize = r.size();
                for(int i=0; i<rsize; i++)
                {
                    std::cout << r[i];
                    if(i != rsize-1)
                    {
                        std::cout << ",";
                    }
                }
                std::cout << "]" << std::endl;
            }
        }
        return;
    }

    /*
     * for each vertex in p do:
     */
    for(int i=0; i<psize; i++)
    {
        /*
         * since the last line for this loop,
         * so current vertex must be always in p[0]
         */
        int vertex = p[0];
        /*
         * init new r,p and x. Copy all r into new one
         * than append vertex into new_r
         */
        std::vector<int> new_r = r, new_p, new_x;
        new_r.push_back(vertex);

        // new_p = P U N(vertex)
        #pragma omp task shared(new_p) firstprivate(p,vertex)
        new_p = neightbour(vertex,p);

        // new_x = X U N(vertex)
        #pragma omp task shared(new_x) firstprivate(x,vertex)
        new_x = neightbour(vertex,x);

        // rescursive all new values
        #pragma omp taskwait
        bronkerbosch(new_r,new_p,new_x);

        // delete vertex from p and append it into x
        p.erase(p.begin());
        x.push_back(vertex);
    }
    return;
}
