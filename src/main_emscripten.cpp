#include <iostream>
#include <fstream>
#include <cfloat>
#include <cstdlib>
#include <sstream>
#include <queue>
#include <map>
#include <cassert>

#include "Optimizer.h"
#include "Util.h"

#include <emscripten/bind.h>

using namespace emscripten;
using namespace std;

void initExperiment(const std::string &content, Cluster*& rootCluster, Grid*& g, 
		    vector<PolygonalPath>& curves, int gridResolution){
    //load curves
    float xmin, xmax, ymin, ymax, tmin, tmax;
    stringstream content_stream(content);
    Util::loadCurves(content_stream, curves, xmin, xmax, ymin, ymax, tmin, tmax);

    //create grid
    g = new Grid(xmin,ymin,xmax-xmin,ymax-ymin,gridResolution, gridResolution);

    //create root cluster
    rootCluster = new Cluster;
    stringstream ss;
    ss << curves.size();
    rootCluster->name = ss.str();
    rootCluster->parent = NULL;
    Vector* rootVFX = new Vector(gridResolution * gridResolution);
    rootVFX->setValues(0.0);
    Vector* rootVFY = new Vector(gridResolution * gridResolution);
    rootVFY->setValues(0.0);

    rootCluster->vectorField = make_pair(rootVFX, rootVFY);

    for(size_t i = 0 ; i < curves.size() ; ++i){
        rootCluster->indices.push_back(i);
        rootCluster->curveErrors.push_back(0.0f);
    }
}

void saveExperiment(string directory, string currentFileLoaded, Cluster* root){
    stringstream ss;
    ss << directory << "/experiment.txt";

    ofstream experimentFile(ss.str().c_str());
    experimentFile << currentFileLoaded << endl;

    //write hierarchy
    queue<Cluster*> nodesToProcess;

    nodesToProcess.push(root);

    //hack
    map<Cluster*, string> mapClusterPath;
    mapClusterPath[root] = "r";

    experimentFile << "-1 r " << endl;

    while(!nodesToProcess.empty()){
        Cluster* c = nodesToProcess.front();
        nodesToProcess.pop();

        string name = mapClusterPath[c];

        //write curve indices file
        stringstream curveFileName;
        curveFileName << directory << "/curves_" << name << ".txt";
        ofstream curveIndicesFile(curveFileName.str().c_str());
        int numberOfCurves = c->indices.size();

        //cout << "NumCurves " << numberOfCurves << " errors " << c->curveErrors.size() << endl;
        assert(numberOfCurves == (int)c->curveErrors.size());

        for(int i = 0 ; i < numberOfCurves ; ++i){
            curveIndicesFile << c->indices.at(i) << " " << c->curveErrors.at(i) << endl;
        }
        curveIndicesFile.close();

        //write vector field file file
        stringstream vectorFieldFileName;
        vectorFieldFileName << directory << "/vf_" << name << ".txt";
        ofstream vectorFieldFile(vectorFieldFileName.str().c_str());
        Vector* xComponent = c->vectorField.first;
        Vector* yComponent = c->vectorField.second;
        int gridDimension = xComponent->getDimension();

        vectorFieldFile << gridDimension << endl;

        for(int i = 0 ; i < gridDimension ; ++i){
            vectorFieldFile << xComponent[0][i] << " " << yComponent[0][i] << endl;
        }
        vectorFieldFile.close();

        //process children
        int numberOfChildren = c->children.size();

        for(int i = 0 ; i < numberOfChildren ; ++i){
            Cluster* child = c->children.at(i);

            stringstream ss;
            ss << name << "_" <<  i;
            mapClusterPath[child] = ss.str();

            experimentFile << name << " " << ss.str() << endl;

            nodesToProcess.push(child);
        }
    }

}

StepperState emscripten_main_2(const std::string &trajectories,
                               int gridResolution,
                               int numberOfVectorFields,
                               float smoothnessWeight)
{
    //load files
    cout << "Loading Files..." << endl;
    vector<PolygonalPath> curves;
    Cluster* rootCluster = NULL;
    Grid* g = NULL;

    //
    cout << "Loading data" << endl;
    initExperiment(trajectories, rootCluster, g, curves, gridResolution);

    // //optimize
    cout << "initializing optimizer structures..." << endl;
    Cluster* currentCluster = rootCluster;

    //Optimize
    Optimizer op(g->getResolutionX() * g->getResolutionY());
    int numberOfCurves = currentCluster->indices.size();

    unsigned short mapCurveToVF[numberOfCurves];
    float mapCurveToError[numberOfCurves];
    unsigned int mapCurveToIndexInCurveVector[numberOfCurves];
    vector<float> mapVectorFieldToError;
    vector<PolygonalPath> curvesInCurrentCluster;

    for(int i = 0 ; i <numberOfCurves ; ++i){
        mapCurveToError[i] = 0;
        mapCurveToVF[i] = -1;

        curvesInCurrentCluster.push_back(curves.at(currentCluster->indices.at(i)));
        mapCurveToIndexInCurveVector[i] = currentCluster->indices.at(i);
    }

    return StepperState(g, numberOfVectorFields, smoothnessWeight,
                        curvesInCurrentCluster);
}

EMSCRIPTEN_BINDINGS(my_module)
{
    class_<Vector2D>("Vector2D")
        .constructor<float, float>()
        .function("X", &Vector2D::X)
        .function("Y", &Vector2D::Y)
        ;

    class_<Vector>("Vector")
        .function("getValue", &Vector::getValue)
        .function("getDimension", &Vector::getDimension)
        ;

    value_object<pair<int, double> >("pairIntDouble")
        .field("first", &pair<int,double>::first)
        .field("second", &pair<int,double>::second)
        ;

    value_object<pair<Vector, Vector> >("pairVectorVector")
        .field("first", &pair<Vector,Vector>::first)
        .field("second", &pair<Vector,Vector>::second)
        ;

    class_<StepperState>("StepperState")
        .function("step", &StepperState::step)
        .function("get", &StepperState::get);

    value_object<pair<Vector2D, float> >("pairVector2DFloat")
        .field("first", &pair<Vector2D,float>::first)
        .field("second", &pair<Vector2D,float>::second)
        ;

    class_<PolygonalPath>("PolygonalPath")
        .function("numberOfPoints", &PolygonalPath::numberOfPoints)
        .function("getPoint", &PolygonalPath::getPoint)
        .function("getTangent", &PolygonalPath::getPoint)
        ;

    emscripten::function("init", &emscripten_main_2);
}
