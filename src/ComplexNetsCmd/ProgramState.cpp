// This toolbox is licensed under the Academic Free License 3.0.
// Instituto Tecnológico de Buenos Aires (ITBA).
// Last modification: December 19th, 2012.

#include "ProgramState.h"
#include "ComplexNets/DegreeDistribution.h"
#include "ComplexNets/DirectedDegreeDistribution.h"
#include "ComplexNets/DirectedGraphFactory.h"
#include "ComplexNets/GraphFactory.h"
#include "ComplexNets/GraphGenerator.h"
#include "ComplexNets/GraphWriter.h"
#include "ComplexNets/IBetweenness.h"
#include "ComplexNets/IClusteringCoefficient.h"
#include "ComplexNets/IGraphFactory.h"
#include "ComplexNets/INearestNeighborsDegree.h"
#include "ComplexNets/IShellIndex.h"
#include "ComplexNets/PropertyMap.h"
#include "ComplexNets/WeightedGraphFactory.h"
#include "ComplexNetsGui/inc/GrapherUtils.h"
#include "ComplexNetsGui/inc/LogBinningPolicy.h"

using namespace graphpp;
using namespace ComplexNetsGui;

ProgramState::ProgramState()
{
    setWeighted(false);
}

void ProgramState::setWeighted(bool weighted)
{
    this->weighted = weighted;
}

bool ProgramState::isWeighted()
{
    return this->weighted;
}

void ProgramState::setDigraph(bool digraph)
{
    this->digraph = digraph;
}

bool ProgramState::isDigraph()
{
    return this->digraph;
}

WeightedGraph ProgramState::getWeightedGraph()
{
    return this->weightedGraph;
}

DirectedGraph ProgramState::getDirectedGraph()
{
    return this->directedGraph;
}

Graph ProgramState::getGraph()
{
    return this->graph;
}

void ProgramState::readGraphFromFile(std::string path)
{
    if (isWeighted())
    {
        this->weightedGraph =
            *(GraphGenerator::getInstance()->generateWeightedGraphFromFile(path, false, true));
    }
    else if (isDigraph())
    {
        this->directedGraph =
            *(GraphGenerator::getInstance()->generateDirectedGraphFromFile(path, false));
    }
    else
    {
        this->graph = *(GraphGenerator::getInstance()->generateGraphFromFile(path, false, true));
    }
}

void ProgramState::setErdosRenyiGraph(unsigned int n, float p)
{
    setWeighted(false);
    this->graph = *GraphGenerator::getInstance()->generateErdosRenyiGraph(n, p);
}

void ProgramState::setBarabasiAlbertGraph(unsigned int m_0, unsigned int m, unsigned int n)
{
    setWeighted(false);
    this->graph = *GraphGenerator::getInstance()->generateBarabasiAlbertGraph(m_0, m, n);
}

void ProgramState::setExtendedHotGraph(
    unsigned int m, unsigned int n, float xi, unsigned int q, float r, unsigned int t)
{
    setWeighted(false);
    this->graph = *GraphGenerator::getInstance()->generateHotExtendedGraph(m, n, xi, q, r, t);
}

void ProgramState::setMolloyReedGraph(std::string path)
{
    setWeighted(false);
    this->graph = *GraphGenerator::getInstance()->generateMolloyReedGraph(path);
}

void ProgramState::setHiperbolicGraph(unsigned int n, float a, float c)
{
    setWeighted(false);
    std::cout << "Generating hyperbolic graph. Expected avg degree: "
              << GraphGenerator::getInstance()->getExpectedAvgNodeDeg(n, a, c) << std::endl;
    this->graph = *GraphGenerator::getInstance()->generateHiperbolicGraph(n, a, c);
}

double ProgramState::betweenness(unsigned int vertex_id)
{
    IGraphFactory<Graph, Vertex>* factory;
    IBetweenness<Graph, Vertex>* betweenness;
    IGraphFactory<WeightedGraph, WeightedVertex>* wfactory;
    IBetweenness<WeightedGraph, WeightedVertex>* wbetweenness;
    // IBetweenness<Graph, Vertex>::BetweennessIterator it;
    // IBetweenness<WeightedGraph, WeightedVertex>::BetweennessIterator wit;
    double ret;

    if (this->weighted)
    {
        wfactory = new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        wbetweenness = wfactory->createBetweenness(this->weightedGraph);
        IBetweenness<WeightedGraph, WeightedVertex>::BetweennessIterator wit =
            wbetweenness->iterator();
        ret = -1;
        while (!wit.end())
        {
            if (wit->first == vertex_id)
            {
                ret = wit->second;
                break;
            }
            ++wit;
        }
        delete wbetweenness;
    }
    else
    {
        factory = new GraphFactory<Graph, Vertex>();
        betweenness = factory->createBetweenness(this->graph);
        IBetweenness<Graph, Vertex>::BetweennessIterator it = betweenness->iterator();
        ret = -1;
        while (!it.end())
        {
            if (it->first == vertex_id)
            {
                ret = it->second;
                break;
            }
            ++it;
        }
        delete betweenness;
    }

    return ret;
}

std::list<int> ProgramState::maxCliqueAprox()
{
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IMaxClique<Graph, Vertex>* maxClique =
        (IMaxClique<Graph, Vertex>*)factory->createMaxClique(this->graph);

    std::list<int> ret;
    if (maxClique->finished())
    {
        ret = maxClique->getMaxCliqueList();
    }

    delete maxClique;
    return ret;
}

std::list<int> ProgramState::maxCliqueExact(unsigned int max_time)
{
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IMaxClique<Graph, Vertex>* maxClique =
        (IMaxClique<Graph, Vertex>*)factory->createExactMaxClique(this->graph, max_time);

    std::list<int> ret;
    if (maxClique->finished())
    {
        ret = maxClique->getMaxCliqueList();
    }
    delete maxClique;
    return ret;
}

double ProgramState::clustering(unsigned int vertex_id)
{
    double ret = -1;

    if (this->isWeighted())
    {
        WeightedVertex* vertex;
        if ((vertex = weightedGraph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<WeightedGraph, WeightedVertex>* factory =
                new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
            IClusteringCoefficient<WeightedGraph, WeightedVertex>* clusteringCoefficient =
                factory->createClusteringCoefficient();
            ret = clusteringCoefficient->vertexClusteringCoefficient(vertex);
            delete clusteringCoefficient;
        }
    }
    else if (this->isDigraph())
    {
        DirectedVertex* vertex;
        if ((vertex = directedGraph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<DirectedGraph, DirectedVertex>* factory =
                new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
            IClusteringCoefficient<DirectedGraph, DirectedVertex>* clusteringCoefficient =
                factory->createClusteringCoefficient();
            ret = clusteringCoefficient->vertexClusteringCoefficient(
                vertex, directed_out, directed_in);
            delete clusteringCoefficient;
        }
    }
    else
    {
        Vertex* vertex;
        if ((vertex = graph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
            IClusteringCoefficient<Graph, Vertex>* clusteringCoefficient =
                factory->createClusteringCoefficient();
            ret = clusteringCoefficient->vertexClusteringCoefficient(vertex);
            delete clusteringCoefficient;
        }
    }

    return ret;
}

double ProgramState::knn(unsigned int vertex_id)
{
    double ret = -1;
    if (this->isWeighted())
    {
        WeightedVertex* vertex;
        if ((vertex = weightedGraph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<WeightedGraph, WeightedVertex>* factory =
                new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
            INearestNeighborsDegree<WeightedGraph, WeightedVertex>* nearestNeighborsDegree =
                factory->createNearestNeighborsDegree();
            ret = nearestNeighborsDegree->meanDegreeForVertex(vertex);
            delete nearestNeighborsDegree;
        }
    }
    else if (this->isDigraph())
    {
        DirectedVertex* vertex;
        if ((vertex = directedGraph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<DirectedGraph, DirectedVertex>* factory =
                new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
            INearestNeighborsDegree<DirectedGraph, DirectedVertex>* nearestNeighborsDegree =
                factory->createNearestNeighborsDegree();
            ret = nearestNeighborsDegree->meanDegreeForVertex(vertex, directed_out, directed_in);
            delete nearestNeighborsDegree;
        }
    }
    else
    {
        Vertex* vertex;
        if ((vertex = graph.getVertexById(vertex_id)) != nullptr)
        {
            IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
            INearestNeighborsDegree<Graph, Vertex>* nearestNeighborsDegree =
                factory->createNearestNeighborsDegree();
            ret = nearestNeighborsDegree->meanDegreeForVertex(vertex);
            delete nearestNeighborsDegree;
        }
    }
    return ret;
}

double ProgramState::shellIndex(unsigned int vertex_id)
{
    double ret = -1;
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IShellIndex<Graph, Vertex>* shellIndex = factory->createShellIndex(graph);
    IShellIndex<Graph, Vertex>::ShellIndexIterator it = shellIndex->iterator();
    while (!it.end())
    {
        if (it->first == vertex_id)
        {
            ret = it->second;
            break;
        }
        ++it;
    }
    delete shellIndex;
    return ret;
}

double ProgramState::degreeDistribution(unsigned int vertex_id)
{
    double ret = -1;
    if (this->isWeighted())
    {
        IGraphFactory<WeightedGraph, WeightedVertex>* factory =
            new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        IDegreeDistribution<WeightedGraph, WeightedVertex>* degreeDistribution =
            factory->createDegreeDistribution(this->weightedGraph);
        DegreeDistribution<WeightedGraph, WeightedVertex>::DistributionIterator it =
            degreeDistribution->iterator();
        while (!it.end())
        {
            if (it->first == vertex_id)
            {
                ret = it->second;
                break;
            }
            ++it;
        }
        delete degreeDistribution;
    }
    else
    {
        IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
        IDegreeDistribution<Graph, Vertex>* degreeDistribution =
            factory->createDegreeDistribution(this->graph);
        DegreeDistribution<Graph, Vertex>::DistributionIterator it = degreeDistribution->iterator();
        while (!it.end())
        {
            if (it->first == vertex_id)
            {
                ret = it->second;
                break;
            }
            ++it;
        }
        delete degreeDistribution;
    }
    return ret;
}

double ProgramState::inDegreeDistribution(unsigned int vertex_id)
{
    double ret = -1;
    if (this->isDigraph())
    {
        IGraphFactory<DirectedGraph, DirectedVertex>* directedFactory =
            new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>* degreeDistribution =
            static_cast<DirectedDegreeDistribution<DirectedGraph, DirectedVertex>*>(
                directedFactory->createDegreeDistribution(directedGraph));
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>::DistributionIterator it =
            degreeDistribution->inDegreeIterator();
        while (!it.end())
        {
            if (it->first == vertex_id)
            {
                ret = it->second;
                break;
            }
            ++it;
        }
        delete degreeDistribution;
    }
    return ret;
}

double ProgramState::outDegreeDistribution(unsigned int vertex_id)
{
    double ret = -1;
    if (this->isDigraph())
    {
        IGraphFactory<DirectedGraph, DirectedVertex>* directedFactory =
            new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>* degreeDistribution =
            static_cast<DirectedDegreeDistribution<DirectedGraph, DirectedVertex>*>(
                directedFactory->createDegreeDistribution(directedGraph));
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>::DistributionIterator it =
            degreeDistribution->outDegreeIterator();
        while (!it.end())
        {
            if (it->first == vertex_id)
            {
                ret = it->second;
                break;
            }
            ++it;
        }
        delete degreeDistribution;
    }
    return ret;
}

void ProgramState::computeBetweenness(PropertyMap& propertyMap)
{
    IGraphFactory<Graph, Vertex>* factory;
    IBetweenness<Graph, Vertex>* betweenness;
    IGraphFactory<WeightedGraph, WeightedVertex>* wfactory;
    IBetweenness<WeightedGraph, WeightedVertex>* wbetweenness;
    IDegreeDistribution<WeightedGraph, WeightedVertex>* wdegreeDistribution;
    IDegreeDistribution<Graph, Vertex>* degreeDistribution;

    // Calculate betweenness.
    if (this->weighted)
    {
        wfactory = new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        wbetweenness = wfactory->createBetweenness(this->weightedGraph);
        IBetweenness<WeightedGraph, WeightedVertex>::BetweennessIterator betweennessIterator =
            wbetweenness->iterator();
        while (!betweennessIterator.end())
        {
            propertyMap.addProperty<double>(
                "betweenness", to_string<unsigned int>(betweennessIterator->first),
                betweennessIterator->second);
            ++betweennessIterator;
        }
        delete wbetweenness;
    }
    else
    {
        factory = new GraphFactory<Graph, Vertex>();
        betweenness = factory->createBetweenness(this->graph);
        IBetweenness<Graph, Vertex>::BetweennessIterator betweennessIterator =
            betweenness->iterator();
        while (!betweennessIterator.end())
        {
            propertyMap.addProperty<double>(
                "betweenness", to_string<unsigned int>(betweennessIterator->first),
                betweennessIterator->second);
            ++betweennessIterator;
        }
        delete betweenness;
    }

    // Calculate degree distribution.
    if (this->weighted)
    {
        wdegreeDistribution = wfactory->createDegreeDistribution(this->weightedGraph);
        DegreeDistribution<WeightedGraph, WeightedVertex>::DistributionIterator degreeIterator =
            wdegreeDistribution->iterator();
        while (!degreeIterator.end())
        {
            propertyMap.addProperty<double>(
                "degreeDistribution", to_string<unsigned int>(degreeIterator->first),
                degreeIterator->second);
            propertyMap.addProperty<double>(
                "degreeDistributionProbability", to_string<unsigned int>(degreeIterator->first),
                degreeIterator->second / (double)graph.verticesCount());
            ++degreeIterator;
        }
        delete degreeDistribution;
    }
    else
    {
        degreeDistribution = factory->createDegreeDistribution(graph);
        DegreeDistribution<Graph, Vertex>::DistributionIterator degreeIterator =
            degreeDistribution->iterator();
        while (!degreeIterator.end())
        {
            propertyMap.addProperty<double>(
                "degreeDistribution", to_string<unsigned int>(degreeIterator->first),
                degreeIterator->second);
            propertyMap.addProperty<double>(
                "degreeDistributionProbability", to_string<unsigned int>(degreeIterator->first),
                degreeIterator->second / (double)graph.verticesCount());
            ++degreeIterator;
        }
        delete degreeDistribution;
    }

    double betweennessAuxAcum;
    unsigned int degreeAmount;
    VariantsSet betweennessVsDegree;
    VariantsSet& betweennessSet = propertyMap.getPropertySet("betweenness");
    VariantsSet& degreeDistributionSet = propertyMap.getPropertySet("degreeDistribution");
    VariantsSet::const_iterator it = degreeDistributionSet.begin();
    VariantsSet::const_iterator betwennessIt = betweennessSet.begin();
    VariantsSet::const_iterator betweennessVsDegreeIt;
    while (it != degreeDistributionSet.end())
    {
        betweennessVsDegree.insert<double>(it->first, 0.0);
        ++it;
    }
    while (betwennessIt != betweennessSet.end())
    {
        unsigned int vertedId = from_string<unsigned int>(betwennessIt->first);
        // Vertex* v = graph.getVertexById(vertedId);
        Vertex* v =
            this->weighted ? weightedGraph.getVertexById(vertedId) : graph.getVertexById(vertedId);
        betweennessAuxAcum =
            betweennessVsDegree.get_element<double>(to_string<unsigned int>(v->degree()));
        betweennessVsDegree.insert<double>(
            to_string<unsigned int>(v->degree()),
            betweennessAuxAcum + from_string<unsigned int>(betwennessIt->second));

        ++betwennessIt;
    }

    betweennessVsDegreeIt = betweennessVsDegree.begin();
    while (betweennessVsDegreeIt != betweennessVsDegree.end())
    {
        std::string degree = betweennessVsDegreeIt->first;
        betweennessAuxAcum = from_string<double>(betweennessVsDegreeIt->second);
        degreeAmount = degreeDistributionSet.get_element<unsigned int>(degree);

        propertyMap.addProperty<double>(
            "betweennessVsDegree", degree, betweennessAuxAcum / (double)degreeAmount);
        ++betweennessVsDegreeIt;
    }
}

void ProgramState::computeDegreeDistribution(PropertyMap& propertyMap)
{
    // FIXME This should output degreeProbability in the same way that the GUI code does. Check code
    // of digraph DegreeDistribution
    if (isWeighted())
    {
        IGraphFactory<WeightedGraph, WeightedVertex>* weightedFactory =
            new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        IDegreeDistribution<WeightedGraph, WeightedVertex>* degreeDistribution =
            weightedFactory->createDegreeDistribution(this->weightedGraph);
        DegreeDistribution<WeightedGraph, WeightedVertex>::DistributionIterator it =
            degreeDistribution->iterator();

        while (!it.end())
        {
            propertyMap.addProperty<double>(
                "degreeDistribution", to_string<unsigned int>(it->first), it->second);
            propertyMap.addProperty<double>(
                "degreeDistributionProbability", to_string<unsigned int>(it->first),
                it->second / (double)this->weightedGraph.verticesCount());
            ++it;
        }

        delete degreeDistribution;
    }
    else if (isDigraph())
    {
        IGraphFactory<DirectedGraph, DirectedVertex>* directedFactory =
            new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>* degreeDistribution =
            static_cast<DirectedDegreeDistribution<DirectedGraph, DirectedVertex>*>(
                directedFactory->createDegreeDistribution(directedGraph));
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>::DistributionIterator it =
            degreeDistribution->inDegreeIterator();
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>::DistributionIterator it2 =
            degreeDistribution->outDegreeIterator();
        DirectedDegreeDistribution<DirectedGraph, DirectedVertex>::DistributionIterator it3 =
            degreeDistribution->inOutDegreeIterator();

        while (!it.end() || !it2.end())
        {
            if (!it.end())
            {
                propertyMap.addProperty<double>(
                    "inDegreeDistribution", to_string<unsigned int>(it->first), it->second);
                propertyMap.addProperty<double>(
                    "inDegreeDistributionProbability", to_string<unsigned int>(it->first),
                    it->second / (double)directedGraph.verticesCount());
                ++it;
            }
            if (!it2.end())
            {
                propertyMap.addProperty<double>(
                    "outDegreeDistribution", to_string<unsigned int>(it2->first), it2->second);
                propertyMap.addProperty<double>(
                    "outDegreeDistributionProbability", to_string<unsigned int>(it2->first),
                    it2->second / (double)directedGraph.verticesCount());
                ++it2;
            }
            if (!it3.end())
            {
                propertyMap.addProperty<double>(
                    "inOutDegreeDistribution", to_string<unsigned int>(it3->first), it3->second);
                propertyMap.addProperty<double>(
                    "inOutDegreeDistributionProbability", to_string<unsigned int>(it3->first),
                    it3->second / (double)directedGraph.verticesCount());
                ++it3;
            }
        }

        delete degreeDistribution;
    }
    else
    {
        IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
        IDegreeDistribution<Graph, Vertex>* degreeDistribution =
            factory->createDegreeDistribution(this->graph);
        DegreeDistribution<Graph, Vertex>::DistributionIterator it = degreeDistribution->iterator();

        while (!it.end())
        {
            propertyMap.addProperty<double>(
                "degreeDistribution", to_string<unsigned int>(it->first), it->second);
            propertyMap.addProperty<double>(
                "degreeDistributionProbability", to_string<unsigned int>(it->first),
                it->second / (double)this->graph.verticesCount());
            ++it;
        }

        delete degreeDistribution;
    }
}

void ProgramState::computeClusteringCoefficient(PropertyMap& propertyMap)
{
    if (!propertyMap.containsPropertySet("degreeDistribution"))
    {
        computeDegreeDistribution(propertyMap);
    }

    std::string key = "degreeDistribution";
    if (isDigraph())
    {
        if (directed_in && directed_out)
        {
            key = "inOutDegreeDistribution";
        }
        else if (directed_in)
        {
            key = "inDegreeDistribution";
        }
        else if (directed_out)
        {
            key = "outDegreeDistribution";
        }
    }

    VariantsSet& degrees = propertyMap.getPropertySet(key);
    VariantsSet::const_iterator it = degrees.begin();

    double cc = 0;

    if (isWeighted())
    {
        IGraphFactory<WeightedGraph, WeightedVertex>* weightedFactory =
            new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        IClusteringCoefficient<WeightedGraph, WeightedVertex>* clusteringCoefficient =
            weightedFactory->createClusteringCoefficient();
        while (it != degrees.end())
        {
            cc = clusteringCoefficient->clusteringCoefficient(
                weightedGraph, from_string<unsigned int>(it->first));
            propertyMap.addProperty<double>("clusteringCoeficientForDegree", it->first, cc);
            ++it;
        }

        delete clusteringCoefficient;
    }
    else if (isDigraph())
    {
        IGraphFactory<DirectedGraph, DirectedVertex>* directedFactory =
            new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
        IClusteringCoefficient<DirectedGraph, DirectedVertex>* clusteringCoefficient =
            directedFactory->createClusteringCoefficient();
        while (it != degrees.end())
        {
            cc = clusteringCoefficient->clusteringCoefficient(
                directedGraph, from_string<unsigned int>(it->first), directed_out, directed_in);
            propertyMap.addProperty<double>(
                "clusteringCoeficientForDegree" + getDirectedPostfix(), it->first, cc);
            ++it;
        }

        delete clusteringCoefficient;
    }
    else
    {
        IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
        IClusteringCoefficient<Graph, Vertex>* clusteringCoefficient =
            factory->createClusteringCoefficient();
        while (it != degrees.end())
        {
            cc = clusteringCoefficient->clusteringCoefficient(
                graph, from_string<unsigned int>(it->first));
            propertyMap.addProperty<double>("clusteringCoeficientForDegree", it->first, cc);
            ++it;
        }

        delete clusteringCoefficient;
    }
}

void ProgramState::computeNearestNeighborsDegree(PropertyMap& propertyMap)
{
    if (!propertyMap.containsPropertySet("degreeDistribution"))
    {
        computeDegreeDistribution(propertyMap);
    }

    std::string key = "degreeDistribution";
    if (isDigraph())
    {
        if (directed_in && directed_out)
        {
            key = "inOutDegreeDistribution";
        }
        else if (directed_in)
        {
            key = "inDegreeDistribution";
        }
        else if (directed_out)
        {
            key = "outDegreeDistribution";
        }
    }

    VariantsSet& degrees = propertyMap.getPropertySet(key);
    VariantsSet::const_iterator it = degrees.begin();

    double knn = 0;

    if (isWeighted())
    {
        IGraphFactory<WeightedGraph, WeightedVertex>* weightedFactory =
            new WeightedGraphFactory<WeightedGraph, WeightedVertex>();
        INearestNeighborsDegree<WeightedGraph, WeightedVertex>* nearestNeighborDegree =
            weightedFactory->createNearestNeighborsDegree();

        while (it != degrees.end())
        {
            knn = nearestNeighborDegree->meanDegree(
                weightedGraph, from_string<unsigned int>(it->first));
            propertyMap.addProperty<double>("nearestNeighborDegreeForDegree", it->first, knn);
            ++it;
        }

        delete nearestNeighborDegree;
    }
    else if (isDigraph())
    {
        IGraphFactory<DirectedGraph, DirectedVertex>* directedFactory =
            new DirectedGraphFactory<DirectedGraph, DirectedVertex>();
        INearestNeighborsDegree<DirectedGraph, DirectedVertex>* nearestNeighborDegree =
            directedFactory->createNearestNeighborsDegree();

        while (it != degrees.end())
        {
            knn = nearestNeighborDegree->meanDegree(
                directedGraph, from_string<unsigned int>(it->first), directed_out, directed_in);
            propertyMap.addProperty<double>(
                "nearestNeighborDegreeForDegree" + getDirectedPostfix(), it->first, knn);
            ++it;
        }

        delete nearestNeighborDegree;
    }
    else
    {
        IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
        INearestNeighborsDegree<Graph, Vertex>* nearestNeighborDegree =
            factory->createNearestNeighborsDegree();

        while (it != degrees.end())
        {
            knn = nearestNeighborDegree->meanDegree(graph, from_string<unsigned int>(it->first));
            propertyMap.addProperty<double>("nearestNeighborDegreeForDegree", it->first, knn);
            ++it;
        }

        delete nearestNeighborDegree;
    }
}

void ProgramState::computeShellIndex(PropertyMap& propertyMap)
{
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IShellIndex<Graph, Vertex>* shellIndex = factory->createShellIndex(graph);
    IShellIndex<Graph, Vertex>::ShellIndexIterator it = shellIndex->iterator();

    while (!it.end())
    {
        propertyMap.addProperty<unsigned int>(
            "shellIndex", to_string<unsigned int>(it->first), it->second);
        ++it;
    }

    delete shellIndex;
}

bool ProgramState::computeMaxCliqueDistr(
    PropertyMap& propertyMap, bool exact, unsigned int max_time)
{
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IMaxClique<Graph, Vertex>* maxClique =
        exact ? (IMaxClique<Graph, Vertex>*)factory->createExactMaxClique(this->graph, max_time)
              : (IMaxClique<Graph, Vertex>*)factory->createMaxClique(this->graph);

    if (maxClique->finished())
    {
        DistributionIterator it = maxClique->distIterator();
        while (!it.end())
        {
            propertyMap.addProperty<double>(
                exact ? "maxCliqueExactDistribution" : "maxCliqueAproxDistribution",
                to_string<int>(it->first), it->second);
            it++;
        }
    }
    return maxClique->finished();
}

bool ProgramState::exportMaxCliqueExact(std::string outputPath, unsigned int max_time)
{
    PropertyMap propertyMap;
    if (computeMaxCliqueDistr(propertyMap, true, max_time))
    {
        GrapherUtils utils;
        utils.exportPropertySet(
            propertyMap.getPropertySet("maxCliqueExactDistribution"), outputPath);
        return true;
    }
    return false;
}

void ProgramState::exportMaxCliqueAprox(std::string outputPath)
{
    PropertyMap propertyMap;
    computeMaxCliqueDistr(propertyMap, false, 0);
    GrapherUtils utils;
    utils.exportPropertySet(propertyMap.getPropertySet("maxCliqueAproxDistribution"), outputPath);
}

void ProgramState::printDegrees()
{
    std::cout << "Degrees:" << std::endl;
    for (unsigned int i = 1; i <= graph.verticesCount(); ++i)
    {
        Vertex* cur = graph.getVertexById(i);
        std::cout << cur->degree();
        if (i != graph.verticesCount())
        {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

void ProgramState::exportBetweennessVsDegree(std::string outputPath)
{
    PropertyMap propertyMap;
    computeBetweenness(propertyMap);
    GrapherUtils utils;
    utils.exportPropertySet(propertyMap.getPropertySet("betweennessVsDegree"), outputPath);
}

void ProgramState::exportDegreeDistribution(
    std::string outputPath, unsigned int log_bin_given, unsigned int binsAmount)
{
    PropertyMap propertyMap;
    computeDegreeDistribution(propertyMap);
    GrapherUtils grapherUtils;
    if (log_bin_given)
    {
        if (!isDigraph())
        {
            VariantsSet& set = propertyMap.getPropertySet("degreeDistribution");
            LogBinningPolicy policy;
            grapherUtils.exportPropertySet(policy.transform(set, binsAmount), outputPath);
        }
        else
        {
            VariantsSet& set = propertyMap.getPropertySet("inDistributionProbability");
            LogBinningPolicy policy;
            std::string outputPath1 = outputPath;
            std::string outputPath2 = outputPath;
            grapherUtils.exportPropertySet(
                policy.transform(set, binsAmount), outputPath1.append("_in_degree"));

            VariantsSet& set2 = propertyMap.getPropertySet("outDistributionProbability");
            grapherUtils.exportPropertySet(
                policy.transform(set2, binsAmount), outputPath2.append("_out_degree"));
        }
        return;
    }
    else
    {
        if (!isDigraph())
        {
            VariantsSet& set = propertyMap.getPropertySet("degreeDistributionProbability");
            grapherUtils.exportPropertySet(set, outputPath);
        }
        else
        {
            std::string outputPath1 = outputPath;
            std::string outputPath2 = outputPath;
            grapherUtils.exportPropertySet(
                propertyMap.getPropertySet("inDegreeDistributionProbability"),
                outputPath1.append("_in_degree"));
            grapherUtils.exportPropertySet(
                propertyMap.getPropertySet("outDegreeDistributionProbability"),
                outputPath2.append("_out_degree"));
        }
    }
}

void ProgramState::exportClusteringVsDegree(std::string outputPath)
{
    PropertyMap propertyMap;
    computeClusteringCoefficient(propertyMap);
    GrapherUtils grapherUtils;
    grapherUtils.exportPropertySet(
        propertyMap.getPropertySet("clusteringCoeficientForDegree" + getDirectedPostfix()),
        outputPath);
}

std::string ProgramState::getDirectedPostfix()
{
    std::string directedPostfix;
    if (directed_out)
    {
        directedPostfix += "O";
    }
    if (directed_in)
    {
        directedPostfix += "I";
    }
    return directedPostfix;
}

void ProgramState::exportNearestNeighborsDegreeVsDegree(std::string outputPath)
{
    PropertyMap propertyMap;
    computeNearestNeighborsDegree(propertyMap);
    GrapherUtils grapherUtils;
    grapherUtils.exportPropertySet(
        propertyMap.getPropertySet("nearestNeighborDegreeForDegree" + getDirectedPostfix()),
        outputPath);
}

void ProgramState::exportShellIndexVsDegree(std::string outputPath)
{
    PropertyMap propertyMap;
    computeShellIndex(propertyMap);
    computeDegreeDistribution(propertyMap);

    double shellAuxAcum;
    unsigned int degreeAmount, vertexId;

    VariantsSet shellIndexVsDegree;
    VariantsSet& shellIndex = propertyMap.getPropertySet("shellIndex");
    VariantsSet& degreeDistribution = propertyMap.getPropertySet("degreeDistribution");
    VariantsSet::const_iterator it = degreeDistribution.begin();
    VariantsSet::const_iterator shellIt = shellIndex.begin();
    VariantsSet::const_iterator shellVsDegreeIt;

    while (it != degreeDistribution.end())
    {
        shellIndexVsDegree.insert<double>(it->first, 0);
        ++it;
    }

    while (shellIt != shellIndex.end())
    {
        vertexId = from_string<unsigned int>(shellIt->first);
        Vertex* v = graph.getVertexById(vertexId);
        shellAuxAcum = shellIndexVsDegree.get_element<double>(to_string<unsigned int>(v->degree()));
        shellIndexVsDegree.insert<double>(
            to_string<unsigned int>(v->degree()),
            shellAuxAcum + from_string<unsigned int>(shellIt->second));

        ++shellIt;
    }

    shellVsDegreeIt = shellIndexVsDegree.begin();
    while (shellVsDegreeIt != shellIndexVsDegree.end())
    {
        std::string degree = shellVsDegreeIt->first;
        shellAuxAcum = from_string<double>(shellVsDegreeIt->second);
        degreeAmount = degreeDistribution.get_element<unsigned int>(degree);

        shellIndexVsDegree.insert<double>(degree, shellAuxAcum / (double)degreeAmount);
        ++shellVsDegreeIt;
    }

    GrapherUtils grapherUtils;
    grapherUtils.exportPropertySet(shellIndexVsDegree, outputPath);
}

void ProgramState::exportCurrentGraph(std::string outputPath)
{
    GraphWriter* graphWriter = new GraphWriter();

    if (isWeighted())
    {
        graphWriter->writeWeightedGraph(&(this->weightedGraph), outputPath);
    }
    else if (isDigraph())
    {
        graphWriter->writeDirectedGraph(&(this->directedGraph), outputPath);
    }
    else
    {
        graphWriter->writeGraph(&(this->graph), outputPath);
    }

    delete graphWriter;
}

void ProgramState::exportCCBoxplot(std::string outputPath)
{
    PropertyMap propertyMap;
    computeNearestNeighborsDegree(propertyMap);
    GrapherUtils grapherUtils;
    grapherUtils.exportPropertySet(
        propertyMap.getPropertySet("nearestNeighborDegreeForDegree"), outputPath);
}

void ProgramState::setDirectedInOut(bool o, bool i)
{
    directed_out = o;
    directed_in = i;
}

graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry
ProgramState::computeTotalBpEntriesDegreeDistribution()
{
    Graph& g = graph;
    Graph::VerticesIterator vit = g.verticesIterator();
    std::vector<int> bCoefs;
    //  IBetweenness<Graph, Vertex>* betweenness = factory->createBetweenness(g);
    double coefSums = 0.0;
    unsigned int count = 0;

    while (!vit.end())
    {
        Vertex* v = *vit;
        int c = v->degree();
        bCoefs.push_back(c);
        coefSums += c;
        ++vit;
        count++;
    }
    std::sort(bCoefs.begin(), bCoefs.end());
    graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry entry;
    if (bCoefs.size() > 0)
    {
        entry.mean = count == 0 ? 0 : coefSums / count;
        entry.min = bCoefs.front();
        entry.max = bCoefs.back();
        int const Q1 = bCoefs.size() / 4;
        int const Q2 = bCoefs.size() / 2;
        int const Q3 = bCoefs.size() * (0.75);
        entry.Q1 = bCoefs.at(Q1);
        entry.Q2 = bCoefs.at(Q2);
        entry.Q3 = bCoefs.at(Q3);
        for (int t = 0; t < bCoefs.size(); t++)
        {
            entry.values.push_back(bCoefs[t]);
        }
    }
    bCoefs.clear();
    return entry;
}

graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry
ProgramState::computeTotalBpEntriesShellIndex()
{
    Graph& g = graph;
    Graph::VerticesIterator vit = g.verticesIterator();
    std::vector<double> bCoefs;
    PropertyMap propertyMap;
    computeShellIndex(propertyMap);
    // IShellIndex<Graph, Vertex>* betweenness = factory->createShellIndex(g);
    double coefSums = 0.0;
    unsigned int count = 0;

    while (!vit.end())
    {
        Vertex* v = *vit;
        int c =
            propertyMap.getProperty<int>("shellIndex", to_string<unsigned int>(v->getVertexId()));
        bCoefs.push_back(c);
        coefSums += c;
        ++vit;
        count++;
    }
    std::sort(bCoefs.begin(), bCoefs.end());
    graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry entry;
    if (bCoefs.size() > 0)
    {
        entry.mean = count == 0 ? 0 : coefSums / count;
        entry.min = bCoefs.front();
        entry.max = bCoefs.back();
        int const Q1 = bCoefs.size() / 4;
        int const Q2 = bCoefs.size() / 2;
        int const Q3 = bCoefs.size() * (0.75);
        entry.Q1 = bCoefs.at(Q1);
        entry.Q2 = bCoefs.at(Q2);
        entry.Q3 = bCoefs.at(Q3);
        for (int t = 0; t < bCoefs.size(); t++)
        {
            entry.values.push_back(bCoefs[t]);
        }
    }
    bCoefs.clear();
    return entry;
}

graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry
ProgramState::computeTotalBpEntriesBetweenness()
{
    Graph& g = graph;
    Graph::VerticesIterator vit = g.verticesIterator();
    std::vector<double> bCoefs;
    PropertyMap propertyMap;
    computeBetweenness(propertyMap);
    // IBetweenness<Graph, Vertex>* betweenness = factory->createBetweenness(g);
    double coefSums = 0.0;
    unsigned int count = 0;

    while (!vit.end())
    {
        Vertex* v = *vit;
        double c = propertyMap.getProperty<double>(
            "betweenness", to_string<unsigned int>(v->getVertexId()));
        bCoefs.push_back(c);
        coefSums += c;
        ++vit;
        count++;
    }
    std::sort(bCoefs.begin(), bCoefs.end());
    graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry entry;
    if (bCoefs.size() > 0)
    {
        entry.mean = count == 0 ? 0 : coefSums / count;
        entry.min = bCoefs.front();
        entry.max = bCoefs.back();
        int const Q1 = bCoefs.size() / 4;
        int const Q2 = bCoefs.size() / 2;
        int const Q3 = bCoefs.size() * (0.75);
        entry.Q1 = bCoefs.at(Q1);
        entry.Q2 = bCoefs.at(Q2);
        entry.Q3 = bCoefs.at(Q3);
        for (int t = 0; t < bCoefs.size(); t++)
        {
            entry.values.push_back(bCoefs[t]);
        }
    }
    bCoefs.clear();
    return entry;
}

graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry
ProgramState::computeTotalBpEntriesKnn()
{
    Graph& g = graph;
    Graph::VerticesIterator vit = g.verticesIterator();
    std::vector<graphpp::INearestNeighborsDegree<Graph, Vertex>::MeanDegree> nnCoefs;
    PropertyMap propertyMap;
    computeDegreeDistribution(propertyMap);
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    INearestNeighborsDegree<Graph, Vertex>* nearestNeighborDegree =
        factory->createNearestNeighborsDegree();
    double coefSums = 0.0;
    unsigned int count = 0;

    double oldCoef;
    int degree_exists = propertyMap.containsPropertySet("nearestNeighborDegreeForDegreeO") ? 1 : 0;

    while (!vit.end())
    {
        Vertex* v = *vit;
        if (!propertyMap.containsProperty(
                "nearestNeighborDegreeForDegreeO", to_string<unsigned int>(v->degree())))
        {
            oldCoef = 0;
        }
        else
        {
            oldCoef = propertyMap.getProperty<double>(
                "nearestNeighborDegreeForDegreeO", to_string<unsigned int>(v->degree()));
        }
        graphpp::INearestNeighborsDegree<Graph, Vertex>::MeanDegree c =
            nearestNeighborDegree->meanDegreeForVertex(v);
        propertyMap.addProperty<double>(
            "nearestNeighborDegreeForVertex", to_string<unsigned int>(v->getVertexId()), c);
        if (degree_exists == 0)
            propertyMap.addProperty<double>(
                "nearestNeighborDegreeForDegreeO", to_string<unsigned int>(v->degree()),
                oldCoef + (c /
                           propertyMap.getProperty<double>(
                               "degreeDistribution", to_string<unsigned int>(v->degree()))));
        nnCoefs.push_back(c);
        coefSums += c;
        ++vit;
        count++;
    }
    std::sort(nnCoefs.begin(), nnCoefs.end());
    graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry entry;
    if (nnCoefs.size() > 0)
    {
        entry.mean = count == 0 ? 0 : coefSums / count;
        entry.min = nnCoefs.front();
        entry.max = nnCoefs.back();
        int const Q1 = nnCoefs.size() / 4;
        int const Q2 = nnCoefs.size() / 2;
        int const Q3 = nnCoefs.size() * (0.75);
        entry.Q1 = nnCoefs.at(Q1);
        entry.Q2 = nnCoefs.at(Q2);
        entry.Q3 = nnCoefs.at(Q3);
        for (int t = 0; t < nnCoefs.size(); t++)
        {
            entry.values.push_back(nnCoefs[t]);
        }
    }
    nnCoefs.clear();
    return entry;
}

graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry ProgramState::computeTotalBpEntries()
{
    Graph& g = graph;
    Graph::VerticesIterator vit = g.verticesIterator();
    std::vector<graphpp::IClusteringCoefficient<Graph, Vertex>::Coefficient> clusteringCoefs;
    PropertyMap propertyMap;
    computeDegreeDistribution(propertyMap);
    IGraphFactory<Graph, Vertex>* factory = new GraphFactory<Graph, Vertex>();
    IClusteringCoefficient<Graph, Vertex>* clusteringCoefficient =
        factory->createClusteringCoefficient();
    double coefSums = 0.0;
    unsigned int count = 0;
    double oldCoef;
    int degree_exists = propertyMap.containsPropertySet("clusteringCoeficientForDegreeO") ? 1 : 0;

    while (!vit.end())
    {
        Vertex* v = *vit;
        if (!propertyMap.containsProperty(
                "clusteringCoeficientForDegreeO", to_string<unsigned int>(v->degree())))
        {
            oldCoef = 0;
        }
        else
        {
            oldCoef = propertyMap.getProperty<double>(
                "clusteringCoeficientForDegreeO", to_string<unsigned int>(v->degree()));
        }
        graphpp::IClusteringCoefficient<Graph, Vertex>::Coefficient c =
            clusteringCoefficient->vertexClusteringCoefficient(v);
        propertyMap.addProperty<double>(
            "clusteringCoeficientForVertex", to_string<unsigned int>(v->getVertexId()), c);
        if (degree_exists == 0)
            propertyMap.addProperty<double>(
                "clusteringCoeficientForDegreeO", to_string<unsigned int>(v->degree()),
                oldCoef + (c /
                           propertyMap.getProperty<double>(
                               "degreeDistribution", to_string<unsigned int>(v->degree()))));
        clusteringCoefs.push_back(c);
        coefSums += c;
        ++vit;
        count++;
    }
    std::sort(clusteringCoefs.begin(), clusteringCoefs.end());
    graphpp::IClusteringCoefficient<Graph, Vertex>::Boxplotentry entry;
    if (clusteringCoefs.size() > 0)
    {
        entry.mean = count == 0 ? 0 : coefSums / count;
        entry.min = clusteringCoefs.front();
        entry.max = clusteringCoefs.back();
        int const Q1 = clusteringCoefs.size() / 4;
        int const Q2 = clusteringCoefs.size() / 2;
        int const Q3 = clusteringCoefs.size() * (0.75);
        entry.Q1 = clusteringCoefs.at(Q1);
        entry.Q2 = clusteringCoefs.at(Q2);
        entry.Q3 = clusteringCoefs.at(Q3);
        for (int t = 0; t < clusteringCoefs.size(); t++)
        {
            entry.values.push_back(clusteringCoefs[t]);
        }
    }
    clusteringCoefs.clear();
    return entry;
}
