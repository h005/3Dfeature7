﻿#include "fea.hh"
#include <QtDebug>

#define NoProjection

Fea::Fea(QString modelFile, QString path)
{
    this->path = path;
/*
    // set the output path
    output = matrixFile;

    int pos = output.lastIndexOf('.');
    output.replace(pos,7,".3df");

    std::cout<< "output " << output.toStdString() << std::endl;

    set_tCase();

    setFilenameList_mvpMatrix(matrixFile);

    NUM = fileName.count();

    for(int i = 0; i<NUM ; i++)
        std::cout<<fileName.at(i).toStdString()<<std::endl;
*/
    // the number of feature is 12
    feaArray = new double[12];

    memset(feaArray,0,sizeof(double)*12);

//    qDebug()<<"t_case" << t_case << endl;

    // read in
    exImporter = new ExternalImporter<MyMesh>();

    if(!exImporter->read_mesh(mesh,modelFile.toStdString().c_str()))
    {
        std::cerr << "Error: Cannot read mesh from "<<std::endl;
        return ;
    }

    glm::mat4 tmpPara;
    render = new Render(mesh,tmpPara,tmpPara,tmpPara);

    render->resize(800,800);
    render->show();
}

void Fea::setFeature()
{

    render->setMeshSaliencyPara(exImporter);

    std::vector<MeanCurvature<MyMesh>> a;
    std::vector<GaussCurvature<MyMesh>> b;
    for(int i=0;i<render->p_vecMesh.size();i++)
    {
        MeanCurvature<MyMesh> tmpMean(render->p_vecMesh[i]);
        GaussCurvature<MyMesh> tmpGauss(render->p_vecMesh[i]);
        a.push_back(tmpMean);
        b.push_back(tmpGauss);
    }

    for(; t_case < NUM ; t_case++)
    {
        // render
        render->setMVP(model[t_case],view[t_case],projection[t_case]);

        bool res = render->rendering(t_case);

        if(res)
        {
            render->setParameters();

            //used for check the image
//            render->showImage();

            render->storeImage(path,fileName[t_case]);

            setMat(render->p_img,render->p_width,render->p_height);

            setProjectArea();

            setVisSurfaceArea(render->p_vertices,render->p_VisibleFaces);

            setViewpointEntropy(render->p_verticesMvp,render->p_VisibleFaces);

            setSilhouetteLength();

            setSilhouetteCE();

            setMaxDepth(render->p_img,render->p_height*render->p_width);

            setDepthDistribute(render->p_img,render->p_height*render->p_width);

            setMeanCurvature(t_case,render->p_isVertexVisible,a,render->p_indiceArray);

            setGaussianCurvature(t_case,render->p_isVertexVisible,b,render->p_indiceArray);

            setMeshSaliency(t_case,render->p_vertices,render->p_isVertexVisible,a,render->p_indiceArray);

            setAbovePreference(output,render->p_model);
        }

//        break;
        printOut();

    }

}

void Fea::setMatrixPara(QString matrixPath)
{
    this->matrixPath = matrixPath;

    output = matrixPath;

    int pos = output.lastIndexOf('.');
    output.replace(pos,7,".3df");

    std::cout<< "output " << output.toStdString() << std::endl;

    set_tCase();

    setFilenameList_mvpMatrix(matrixPath);

    NUM = fileName.count();

    for(int i = 0; i<NUM ; i++)
        std::cout<<fileName.at(i).toStdString()<<std::endl;

    std::cout<<t_case<<std::endl;

}


Fea::~Fea()
{

}

void Fea::setMat(float *img, int width, int height)
{
    cv::Mat image0 = cv::Mat(width,height,CV_32FC1,img);
    image0.convertTo(image,CV_8UC1,255.0);
    // release memory
    image0.release();
}

void Fea::setProjectArea()
{
    feaArray[0] = 0.0;
    if(image.channels()==3)
    {
        for(int i=0;i<image.rows;i++)
            for(int j=0;j<image.cols;j++)
                if(image.at<uchar>(i,j,1)!=255
                   || image.at<uchar>(i,j,2)!=255
                   || image.at<uchar>(i,j,3)!=255)
                feaArray[0]++;
    }
    else
    {
        for(int i=0;i<image.rows;i++)
            for(int j=0;j<image.cols;j++)
                if(image.at<uchar>(i,j)!=255)
                    feaArray[0]++;
    }
    std::cout<<"fea projectArea "<<feaArray[0]<<std::endl;
}

void Fea::setVisSurfaceArea(std::vector<GLfloat> &vertex,
                             std::vector<GLuint> &face)
{

    feaArray[1] = 0;
    for(int i=0 ; i<face.size() ; i+=3)
    {
/*
//        area = a*b*sin(ACB)/2
//        the first vertex is A the edge is a(BC)
//        the second vertex is B the edge is b(AC)
//        the third vertex is C the edge is c(AB)
        double a = getDis3D(vertex,face[i+1],face[i+2]);
        double b = getDis3D(vertex,face[i],face[i+2]);
//        double c = getDis(vertex,face(i),face(i+1));
        double cosACB = cosVal3D(vertex,face[i],face[i+1],face[i+2]);
        double sinACB = sqrt(1.0-cosACB*cosACB);
        visSurfaceArea[t_case] += a*b*sinACB/2.0;
*/
        CvPoint3D64f p1 = cvPoint3D64f(vertex[3*face[i]],vertex[3*face[i]+1],vertex[3*face[i]+2]);
        CvPoint3D64f p2 = cvPoint3D64f(vertex[3*face[i+1]],vertex[3*face[i+1]+1],vertex[3*face[i+1]+2]);
        CvPoint3D64f p3 = cvPoint3D64f(vertex[3*face[i+2]],vertex[3*face[i+2]+1],vertex[3*face[i+2]+2]);
        feaArray[1] += getArea3D(&p1,&p2,&p3);
    }

    std::cout<<"fea visSurfaceArea "<< feaArray[1]<<std::endl;
    // used for test
/*
    freopen("vertices.txt","w",stdout);
    for(int i=0;i<vertex.size();i+=3)
        printf("%f %f %f\n",vertex[i],vertex[i+1],vertex[i+2]);
    fclose(stdout);
    freopen("face.txt","w",stdout);
    for(int i=0;i<face.size();i+=3)
        printf("%d %d %d\n",face[i],face[i+1],face[i+2]);
    fclose(stdout);
*/
}

void Fea::setViewpointEntropy(std::vector<GLfloat> &vertex, std::vector<GLuint> &face)
{
    feaArray[2] = 0.0;
    double area = 0.0;
    double totalArea = image.cols*image.rows;
    for(int i=0;i<face.size();i++)
    {
        CvPoint2D64f a = cvPoint2D64f(vertex[face[i]*3],vertex[face[i]*3+1]);
        CvPoint2D64f b = cvPoint2D64f(vertex[face[i+1]*3],vertex[face[i+1]*3+1]);
        CvPoint2D64f c = cvPoint2D64f(vertex[face[i+2]*3],vertex[face[i+2]*3+1]);
        area = getArea2D(&a,&b,&c);
        feaArray[2] += area/totalArea * log2(area/totalArea);
    }
    // background
    feaArray[2] += (totalArea - feaArray[0])/totalArea * log2((totalArea - feaArray[0])/totalArea);

    feaArray[2] = - feaArray[2];
}

void Fea::setSilhouetteLength()
{
    feaArray[3] = 0.0;
//    ref http://blog.csdn.net/augusdi/article/details/9000893
    IplImage *tmpImage =
            cvCreateImage(cvSize(image.cols,image.rows),
                          8,1);
    if(image.channels()==3)
        cv::cvtColor(image,image,CV_BGR2GRAY);

    tmpImage->imageData = (char*)image.data;
    cvThreshold(tmpImage,tmpImage,250,255,CV_THRESH_BINARY_INV);
//    cvShowImage("tmpimage",tmpImage);
    IplImage *img_tmp =
            cvCreateImage(cvGetSize(tmpImage),8,1);
    img_tmp = cvCloneImage(tmpImage);

    CvMemStorage *mem_storage = cvCreateMemStorage(0);

    contour = NULL;
    cvFindContours(
                img_tmp,
                mem_storage,
                &contour,
                sizeof(CvContour),
                CV_RETR_EXTERNAL
                );
    cvZero(img_tmp);
    cvDrawContours(
                img_tmp,
                contour,
                cvScalar(100),
                cvScalar(100),
                1);
//    cvShowImage("image",img_tmp);
//    ref http://blog.csdn.net/fdl19881/article/details/6730112
    if(contour)
        feaArray[3] = cvArcLength(contour);
    else
        feaArray[3] = 0.0;
    std::cout<<"fea silhouetteLength "<<feaArray[3]<<std::endl;
    cvReleaseImage(&tmpImage);
    cvReleaseImage(&img_tmp);
//    cvReleaseMemStorage(&mem_storage);
//    readCvSeqTest(contour);
}

void Fea::setSilhouetteCE()
{
    feaArray[4] = 0.0;
    feaArray[5] = 0.0;
    double curva = 0;
    double dis = 0;
//    example
//    ghabcdefghabcde
//     ^  ->  ^
//    gha -> hab -> abc
    if(contour)
    for(int i=0;i<contour->total;i++)
    {
        CvPoint *a0 = CV_GET_SEQ_ELEM(CvPoint,contour,i-2);
        CvPoint *b0 = CV_GET_SEQ_ELEM(CvPoint,contour,i-1);
        CvPoint *c0 = CV_GET_SEQ_ELEM(CvPoint,contour,i);
        CvPoint2D64f a = cvPoint2D64f((double)a0->x,(double)a0->y);
        CvPoint2D64f b = cvPoint2D64f((double)b0->x,(double)b0->y);
        CvPoint2D64f c = cvPoint2D64f((double)c0->x,(double)c0->y);
//        std::cout<<a0->x<<" "<<a0->y<<std::endl;
//        std::cout<<b0->x<<" "<<b0->y<<std::endl;
//        std::cout<<c0->x<<" "<<c0->y<<std::endl;
        if(getCurvature(&a,&b,&c,curva))
        {
//            std::cout << curva << std::endl;
            dis = getDis2D(&a,&b) + getDis2D(&b,&c);
            feaArray[4] += abs(curva) * dis;
            feaArray[5] += curva*curva * dis;
        }
    }

    std::cout<<"fea silhouetteCurvature "<<feaArray[4]<<std::endl;
    std::cout<<"fea silhouetteCurvatureExtrema "<<feaArray[5]<<std::endl;
/*
    freopen("contour.txt","w",stdout);
    for(int i=0;i<contour->total;i++)
    {
        CvPoint *point = CV_GET_SEQ_ELEM(CvPoint,
                                         contour,i);
        printf("%d %d\n",point->x,point->y);
    }
    fclose(stdout);
*/
}

void Fea::setMaxDepth(float *array,int len)
{
    feaArray[6] = -10.0;
    for(int i=0;i<len;i++)
        if(array[i] < 1.0)
            feaArray[6] = feaArray[6] < array[i] ? feaArray[6] : array[i];
    std::cout<<"fea maxDepth "<<feaArray[6]<<std::endl;
}

void Fea::setDepthDistribute(GLfloat *zBuffer, int num)
{
    feaArray[7] = 0.0;
    double min = 1.0;
    double max = 0.0;
    double *hist = new double[NumHistDepth];
    memset(hist,0,sizeof(double)*NumHistDepth);
    for(int i=0;i<num;i++)
    {
        min = min > zBuffer[i] ? zBuffer[i] : min;
        max = max < zBuffer[i] ? zBuffer[i] : max;
    }
    double step = (max - min)/(double)NumHistDepth;

    if(step)
    {
        // explain for if else below
        // such as min = 0 and max = 15 then step = 1
        // so the hist is [0,1),[1,2),[2,3)...[14,15)
        // max was omit!
        for(int i=0;i<num;i++)
        {
            if(zBuffer[i]==max)
                hist[NumHistDepth - 1]++;
            else
                hist[(int)((zBuffer[i]-min)/step)]++;
        }
        // normalizeHist
        normalizeHist(hist,step,NumHistDepth);
    }

    std::cout<<"step "<<step<<std::endl;
    for(int i=0; i<NumHistDepth; i++)
        feaArray[7] += hist[i]*hist[i]*step;
    feaArray[7] = 1 - feaArray[7];

    std::cout<<"fea depthDistriubute "<<feaArray[7]<<std::endl;
    delete []hist;
//    hist = NULL;
/*
    freopen("depth.txt","w",stdout);
    for(int i=0;i<num;i++)
        printf("%lf\n",zBuffer[i]);
    fclose(stdout);
*/
}

void Fea::setMeanCurvature(MyMesh &mesh, std::vector<bool> &isVertexVisible)
{
    feaArray[8] = 0.0;
    MeanCurvature<MyMesh> a(mesh);
    feaArray[8] = a.getMeanCurvature(isVertexVisible);
    if(feaArray[0])
        feaArray[8] /= feaArray[0];
    std::cout<<"fea meanCurvature "<<feaArray[8]<<std::endl;
}

void Fea::setMeanCurvature(int t_case,
                           std::vector<bool> &isVertexVisible,
                           std::vector<MeanCurvature<MyMesh>> &a,
                           std::vector<std::vector<int>> &indiceArray)
{
    printf("vecMesh....%d\n",a.size());
    printf("indiceArray....%d\n",indiceArray.size());
    feaArray[8] = 0.0;
    for(int i=0;i<a.size();i++)
    {
        // 查看在哪个mesh上面crash掉了
//        std::cout<<"setMeahCurvature.... "<<i<<std::endl;
        // for debug 第136个mesh出错了，输出这个mesh的信息

//        if(i==0)
//        {
//            // 输入文件名即可，outputMesh函数会加上.off后缀名
//            QString tmpPath = this->path;
//            tmpPath.append('/');
//            tmpPath.append(fileName.at(t_case));
//            tmpPath.append(QString::number(i));
//            exImporter->outputMesh(vecMesh[i],tmpPath);
//        }

        std::vector<bool> isVerVis;
//        printf("setMeanCurvature... vertex size %d\n",isVertexVisible.size());
//        printf("setMeanCurvature... indiceArray[%d] size %d\n",i,indiceArray[i].size());
        // 这里有一个bug，这样直接push进去是错的，顶点的数量是5641，而索引的数量是33834
        // 可以写一个set，存在于索引的顶点，按照顺序push进去
        // fixed...
        std::set<int> verIndice;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);

        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            isVerVis.push_back(isVertexVisible[*it]);
//        MeanCurvature<MyMesh> a(vecMesh[i]);
        feaArray[8] += a[i].getMeanCurvature(isVerVis);
    }

    if(feaArray[0])
        feaArray[8] /= feaArray[0];
    std::cout<<"fea meanCurvature "<<feaArray[8]<<std::endl;
}

void Fea::setGaussianCurvature(MyMesh &mesh, std::vector<bool> &isVertexVisible)
{
    feaArray[9] = 0.0;
    GaussCurvature<MyMesh> a(mesh);
    feaArray[9] = a.getGaussianCurvature(isVertexVisible);
    if(feaArray[0])
        feaArray[9] /= feaArray[0];
    std::cout<<"fea gaussianCurvature "<<feaArray[9]<<std::endl;
}

void Fea::setGaussianCurvature(int t_case, // for debug, used for output the mesh
                               std::vector<bool> &isVertexVisible,
                               std::vector<GaussCurvature<MyMesh>> &a,
                               std::vector<std::vector<int>> &indiceArray)
{
    printf("gaussian vecMesh....%d\n",a.size());
    printf("gaussina indiceArray....%d\n",indiceArray.size());
    feaArray[9] = 0.0;
    for(int i=0;i<a.size();i++)
    {
        std::vector<bool> isVerVis;

        std::set<int> verIndice;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);
        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            isVerVis.push_back(isVertexVisible[*it]);

//        GaussCurvature<MyMesh> a(vecMesh[i]);
        feaArray[9] += a[i].getGaussianCurvature(isVerVis);
    }
    if(feaArray[0])
    feaArray[9] /= feaArray[0];
    std::cout<<"fea gaussianCurvature "<<feaArray[9]<<std::endl;
}

void Fea::setMeshSaliency(MyMesh mesh, std::vector<GLfloat> &vertex, std::vector<bool> isVertexVisible)
{
    feaArray[10] = 0.0;
    double length = getDiagonalLength(vertex);
    std::vector<double> meanCurvature;
    double *nearDis = new double[vertex.size()/3];
    MeanCurvature<MyMesh> a(mesh);
    double sigma[5] = {0.003*2.0,0.003*3.0,0.003*4.0,0.003*5.0,0.003*6.0};
    std::vector<double> meshSaliencyMiddle[5];
    double localMax[5];
    double gaussWeightedVal1,gaussWeightedVal2;
    a.setMeanCurvature(meanCurvature);
    for(int j=0;j<5;j++)
    {
        localMax[j] = 0.0;
        for(int i=0;i<vertex.size();i+=3)
        {
            setNearDisMeshSaliency(vertex,i,length,sigma[j],nearDis);
            gaussWeightedVal1 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]);
            gaussWeightedVal2 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]*2.0);
            meshSaliencyMiddle[j].push_back(abs(gaussWeightedVal1 - gaussWeightedVal2));
        }
        double max = meshSaliencyMiddle[j][0];
        double min = meshSaliencyMiddle[j][0];
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
        {
//            global max
            max = max > meshSaliencyMiddle[j][i] ? max : meshSaliencyMiddle[j][i];
//            used for normalize
            min = min > meshSaliencyMiddle[j][i] ? meshSaliencyMiddle[j][i] : min;
//            local max
            setNearDisMeshSaliency(vertex,i*3,length,sigma[j],nearDis);
            localMax[j] += getMeshSaliencyLocalMax(nearDis,vertex.size()/3,meshSaliencyMiddle[j]);
        }
        localMax[j] /= meshSaliencyMiddle[j].size();
//        normalize and set Si
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
            meshSaliencyMiddle[j][i] = (meshSaliencyMiddle[j][i] - min)/(max - min) *
                    (max - localMax[j])*(max - localMax[j]);
    }
//    set sum Si
    for(int i=0;i<meshSaliencyMiddle[0].size();i++)
        for(int j=1;j<5;j++)
            meshSaliencyMiddle[0][i] += meshSaliencyMiddle[j][i];

    for(int i=0;i<isVertexVisible.size();i++)
        if(isVertexVisible[i])
            feaArray[10] += meshSaliencyMiddle[0][i];
//    std::cout<<"fea meshSaliency ";
    printf("fea meshSaliency %e\n",feaArray[10]);
}

void Fea::setMeshSaliency(int t_case,// for debug can be used to output the mesh
                          std::vector<GLfloat> &vertex,
                          std::vector<bool> isVertexVisible,
                          std::vector<MeanCurvature<MyMesh>> &a,
                          std::vector<std::vector<int>> &indiceArray)
{


    feaArray[10] = 0.0;
    double length = getDiagonalLength(vertex);
//    std::vector<double> meanCurvature;
    double *meanCurvature = new double[vertex.size()/3];
    memset(meanCurvature,0,sizeof(double)*vertex.size()/3);
    double *nearDis = new double[vertex.size()/3];

    double sigma[5] = {0.003*2.0,0.003*3.0,0.003*4.0,0.003*5.0,0.003*6.0};
    std::vector<double> meshSaliencyMiddle[5];
    double localMax[5];
    double gaussWeightedVal1,gaussWeightedVal2;

    for(int i=0;i<a.size();i++)
    {
        std::set<int> verIndice;
        std::vector<int> verVec;
        for(int j=0;j<indiceArray[i].size();j++)
            verIndice.insert(indiceArray[i][j]);
        std::set<int>::iterator it = verIndice.begin();
        for(;it!=verIndice.end();it++)
            verVec.push_back(*it);

//        MeanCurvature<MyMesh> a(vecMesh[i]);
        a[i].setMeanCurvature(meanCurvature,verVec);

    }

    printf("set Mesh Saliency.... exImporter done\n");

    for(int j=0;j<5;j++)
    {
        localMax[j] = 0.0;
        for(int i=0;i<vertex.size();i+=3)
        {
            setNearDisMeshSaliency(vertex,i,length,sigma[j],nearDis);
            gaussWeightedVal1 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]);
            gaussWeightedVal2 = getGaussWeightedVal(meanCurvature[i/3],nearDis,vertex.size()/3,sigma[j]*2.0);
            meshSaliencyMiddle[j].push_back(abs(gaussWeightedVal1 - gaussWeightedVal2));
        }
        double max = meshSaliencyMiddle[j][0];
        double min = meshSaliencyMiddle[j][0];
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
        {
//            global max
            max = max > meshSaliencyMiddle[j][i] ? max : meshSaliencyMiddle[j][i];
//            used for normalize
            min = min > meshSaliencyMiddle[j][i] ? meshSaliencyMiddle[j][i] : min;
//            local max
            setNearDisMeshSaliency(vertex,i*3,length,sigma[j],nearDis);
            localMax[j] += getMeshSaliencyLocalMax(nearDis,vertex.size()/3,meshSaliencyMiddle[j]);
        }
        localMax[j] /= meshSaliencyMiddle[j].size();
//        normalize and set Si
        for(int i=0;i<meshSaliencyMiddle[j].size();i++)
            meshSaliencyMiddle[j][i] = (meshSaliencyMiddle[j][i] - min)/(max - min) *
                    (max - localMax[j])*(max - localMax[j]);

        printf("set MeshSaliency .... %d\n",j);
    }

//    set sum Si
    for(int i=0;i<meshSaliencyMiddle[0].size();i++)
        for(int j=1;j<5;j++)
            meshSaliencyMiddle[0][i] += meshSaliencyMiddle[j][i];

    for(int i=0;i<isVertexVisible.size();i++)
        if(isVertexVisible[i])
            feaArray[10] += meshSaliencyMiddle[0][i];
    std::cout<<"fea meshSaliency "<<feaArray[10]<<std::endl;

}

void Fea::setAbovePreference(double theta)
{
    this->feaArray[11] = 0.0;
    double pi = asin(1.0)*2.0;
    feaArray[11] = exp(-(theta - pi/8.0*3.0)*(theta - pi/8.0*3.0)
                          / pi/4.0*pi/4.0);
}

void Fea::setAbovePreference(QString filename, glm::mat4 &model)
{

    int pos = filename.lastIndexOf('.');
    filename.replace(pos,6,".mm");
//    filename.append(QString(".mm"));
    FILE *fp = freopen(filename.toStdString().c_str(),"r",stdin);
    if(fp)
    {
        glm::mat4 model2;
        double tmp;
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
            {
                scanf("%lf",&tmp);
                model2[i][j] = tmp;
            }
//        model2 = glm::transpose(model2);
        glm::vec4 z = glm::vec4(0.0,0.0,1.0,1.0);
        glm::vec4 yyy = model*model2*z;
    //    the theta between yyy and (0,1,0,1)
        double norm_yyy = 0.0;
        glm::vec4 tmp0 = yyy*yyy;
        for(int i=0;i<4;i++)
            norm_yyy += tmp0[i];

        double cosTheta = (yyy.y + 1.0) / sqrt(norm_yyy) / sqrt(2.0);

        double theta = acos(cosTheta);

        setAbovePreference(theta);

        fclose(stdin);
    }
    else
        std::cout<<"fail to open file"<<std::endl;
    std::cout<<"abovePreference "<<feaArray[11]<<std::endl;
}

double Fea::getMeshSaliencyLocalMax(double *nearDis, int len, std::vector<double> meshSaliency)
{
    //可能会有bug,nearDis[0]如果为0的话，赋值是没有意义的
//    double max = meshSaliency[0];
    // meshSaliency >= 0
    double max = 0;
    for(int i=0;i<len;i++)
        if(nearDis[i])
            max = max > meshSaliency[i] ? max : meshSaliency[i];
    return max;
}

double Fea::getGaussWeightedVal(double meanCur, double *nearDis, int len, double sigma)
{
    double numerator = 0.0,denominator = 0.0;
    double expVal = 0.0;
    sigma = 2.0*sigma*sigma;
    for(int i=0;i<len;i++)
    {
        expVal = exp(- nearDis[i] / sigma);
        numerator += meanCur*expVal;
        denominator += expVal;
    }
    return numerator/denominator;
}

double Fea::getDiagonalLength(std::vector<GLfloat> &vertex)
{
//    top t 0
//    bottom b 1  tb
//    left l 0
//    right r 1   lr
//    front f 0
//    behind b 1  fb
//    named as v+lf+tb+fb
//    if vxyz + vpqr = v111 then they are diagonal
    double v_000[3] = {vertex[0],vertex[1],vertex[2]};
    double v_001[3] = {vertex[0],vertex[1],vertex[2]};
    double v_010[3] = {vertex[0],vertex[1],vertex[2]};
    double v_011[3] = {vertex[0],vertex[1],vertex[2]};
    double v_100[3] = {vertex[0],vertex[1],vertex[2]};
    double v_101[3] = {vertex[0],vertex[1],vertex[2]};
    double v_110[3] = {vertex[0],vertex[1],vertex[2]};
    double v_111[3] = {vertex[0],vertex[1],vertex[2]};
    for(int i=3;i<vertex.size();i+=3)
    {
        vertexBoundBox(v_000,vertex,i,0);
        vertexBoundBox(v_001,vertex,i,1);
        vertexBoundBox(v_010,vertex,i,2);
        vertexBoundBox(v_011,vertex,i,3);
        vertexBoundBox(v_100,vertex,i,4);
        vertexBoundBox(v_101,vertex,i,5);
        vertexBoundBox(v_110,vertex,i,6);
        vertexBoundBox(v_111,vertex,i,7);
    }
    double diag[4];
    diag[0] = sqrt((v_000[0]-v_111[0])*(v_000[0]-v_111[0])
            +(v_000[1]-v_111[1])*(v_000[1]-v_111[1])
            +(v_000[2]-v_111[2])*(v_000[2]-v_111[2]));
    diag[1] = sqrt((v_001[0]-v_110[0])*(v_001[0]-v_110[0])
            +(v_001[1]-v_110[1])*(v_001[1]-v_110[1])
            +(v_001[2]-v_110[2])*(v_001[2]-v_110[2]));
    diag[2] = sqrt((v_010[0]-v_101[0])*(v_010[0]-v_101[0])
            +(v_010[1]-v_101[1])*(v_010[1]-v_101[1])
            +(v_010[2]-v_101[2])*(v_010[2]-v_101[2]));
    diag[3] = sqrt((v_011[0]-v_100[0])*(v_011[0]-v_100[0])
            +(v_011[1]-v_100[1])*(v_011[1]-v_100[1])
            +(v_011[2]-v_100[2])*(v_011[2]-v_100[2]));

    double max = 0.0;
    for(int i=0;i<4;i++)
        max = max<diag[i]?diag[i]:max;
    return max;
}

void Fea::setNearDisMeshSaliency(std::vector<GLfloat> &vertex, int index, double len, double sigma, double *nearDis)
{
    double dis = len*2.0*sigma;
    //avoid for sqrt
    dis = dis*dis;
    double disV0V1 = 0.0;
    glm::vec3 v0 = glm::vec3(vertex[index],vertex[index+1],vertex[index+2]);
    glm::vec3 v1;
    for(int i=0;i<vertex.size();i+=3)
    {
        v1 = glm::vec3(vertex[i],vertex[i+1],vertex[i+2]);
        v1 = v1-v0;
        v1 = v1*v1;
        disV0V1 = v1.x+v1.y+v1.z;
        if(disV0V1 > dis)
            nearDis[i/3] = 0.0;
        else
            nearDis[i/3] = disV0V1;
    }
}

void Fea::vertexBoundBox(double *v, std::vector<GLfloat> &vertex, int i, int label)
{
    switch (label) {
    case 0:
        if(vertex[i] >= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 1:
        if(vertex[i] >= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 2:
        if(vertex[i] >= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 3:
        if(vertex[i] >= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 4:
        if(vertex[i] <= v[0] &&
           vertex[i+1] >= v[1] &&
           vertex[i+2] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 5:
        if(vertex[i] <= v[0] &&
           vertex[i] >= v[1] &&
           vertex[i] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 6:
        if(vertex[i] <= v[0] &&
           vertex[i] <= v[1] &&
           vertex[i] >= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    case 7:
        if(vertex[i] <= v[0] &&
           vertex[i+1] <= v[1] &&
           vertex[i+2] <= v[2])
        {
            v[0] = vertex[i];
            v[1] = vertex[i+1];
            v[2] = vertex[i+2];
        }
        break;
    default:
        break;
    }
}

bool Fea::getCurvature(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c, double &cur)
{
    double r = 0;

    if(getR(a,b,c,r))
    {
        cur = 1.0/r;
//        std::cout<<"curvature..."<<cur<<std::endl;
        return true;
    }
    else
        return false;
}

void Fea::readCvSeqTest(CvSeq *seq)
{
    qDebug()<<"readCvSeqTest"<<endl;
    for(int i=0;i<seq->total;i++)
    {
        CvPoint *point = CV_GET_SEQ_ELEM(CvPoint,
                                         seq,i);
        qDebug()<<point->x<<point->y<<endl;
    }
}

double Fea::getArea2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c)
{
    CvPoint2D64f ab = cvPoint2D64f(b->x - a->x, b->y - a->y);
    CvPoint2D64f ac = cvPoint2D64f(c->x - a->x, c->y - a->y);
    double area = ab.x * ac.y - ab.y * ac.x;
    area = area > 0 ? area : -area;
    area /= 2.0;
    return area;
}

double Fea::getArea3D(CvPoint3D64f *a, CvPoint3D64f *b, CvPoint3D64f *c)
{
    CvPoint3D64f ab = cvPoint3D64f(b->x - a->x, b->y - a->y, b->z - a->z);
    CvPoint3D64f ac = cvPoint3D64f(c->x - a->x, c->y - a->y, c->z - a->z);
    double area = (ab.y*ac.z - ac.y*ab.z)*(ab.y*ac.z - ac.y*ab.z)
             + (ac.x*ab.z - ab.x*ac.z)*(ac.x*ab.z - ab.x*ac.z)
            + (ab.x*ac.y - ac.x*ab.y)*(ab.x*ac.y - ac.x*ab.y);
    area = sqrt(area);
    area /= 2.0;
    return area;
}


double Fea::getDis3D(std::vector<float> &vertex,
                   int i1, int i2)
{
    double dx = (vertex[i1*3]-vertex[i2*3]);
    double dy = (vertex[i1*3+1]-vertex[i2*3+1]);
    double dz = (vertex[i1*3+2]-vertex[i2*3+2]);
    return sqrt(dx*dx+dy*dy+dz*dz);
}

double Fea::getDis2D(CvPoint2D64f *a, CvPoint2D64f *b)
{
    double dx = (a->x-b->x);
    double dy = (a->y-b->y);
    return sqrt(dx*dx+dy*dy);
}

double Fea::cosVal3D(std::vector<float> &vertex,
                   int i0, int i1, int i2)
{
    double dotVal = (vertex[i1*3]-vertex[i2*3])*(vertex[i0*3]-vertex[i2*3])
            +(vertex[i1*3+1]-vertex[i2*3+1])*(vertex[i0*3+1]-vertex[i2*3+1])
            +(vertex[i1*3+2]-vertex[i2*3+2])*(vertex[i0*3+2]-vertex[i2*3+2]);
    double va = (vertex[i1*3]-vertex[i2*3])*(vertex[i1*3]-vertex[i2*3])
            +(vertex[i1*3+1]-vertex[i2*3+1])*(vertex[i1*3+1]-vertex[i2*3+1])
            +(vertex[i1*3+2]-vertex[i2*3+2])*(vertex[i1*3+2]-vertex[i2*3+2]);
    va = sqrt(va);
    double vb = (vertex[i0*3]-vertex[i2*3])*(vertex[i0*3]-vertex[i2*3])
            + (vertex[i0*3+1]-vertex[i2*3+1])*(vertex[i0*3+1]-vertex[i2*3+1])
            + (vertex[i0*3+2]-vertex[i2*3+2])*(vertex[i0*3+2]-vertex[i2*3+2]);
    vb = sqrt(vb);
    return dotVal/va/vb;
}

double Fea::cosVal2D(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c)
{
    double dotVal = (a->x-c->x)*(b->x-c->x)
            +(a->y-c->y)*(b->y-c->y);
    double va = (a->x-c->x)*(a->x-c->x)
            +(a->y-c->y)*(a->y-c->y);
    double vb = (b->x-c->x)*(b->x-c->x)
            +(b->y-c->y)*(b->y-c->y);

    va = sqrt(va);
    vb = sqrt(vb);
//    std::cout<<"...cosVal2D "<<dotVal<<" "<<va<<" "<<vb<<std::endl;
    if(!va)
        return 0;
    if(!vb)
        return 0;
    return dotVal/va/vb;
}

bool Fea::getR(CvPoint2D64f *a, CvPoint2D64f *b, CvPoint2D64f *c, double &r)
{
//        area = a*b*sin(ACB)/2
//        area = a*b*sin(ACB)/2
//        the first vertex is A the edge is a(BC)
//        the second vertex is B the edge is b(AC)
//        the third vertex is C the edge is c(AB)
    double c0 = getDis2D(a,b);
    double cosACB = cosVal2D(a,b,c);
    double sinACB = sqrt(1.0 - cosACB*cosACB);
//    std::cout<<c0<<" "<<sinACB<<std::endl;
    if(!sinACB)
        return false;
    r = c0/2.0/sinACB;
    return true;
}

void Fea::normalizeHist(double *hist,double step,int num)
{
    double area = 0.0;
    for(int i=0;i<num;i++)
        area += hist[i]*step;
    for(int i=0;i<num;i++)
        hist[i] /= area;
}

void Fea::initial()
{
}

void Fea::setFilenameList_mvpMatrix(QString matrixFile)
{
    freopen(matrixFile.toStdString().c_str(),"r",stdin);
    QString tmp;
    char tmpss[200];
    float tmpNum;
    while(scanf("%s",tmpss)!=EOF)
    {
        QString tmpPath = path;
        tmp = QDir::cleanPath(tmpPath.append(QString(tmpss)));
        fileName.push_back(tmp);
#ifdef NoProjection
        glm::mat4 m,v;
        for(int i=0;i<16;i++)
        {
            scanf("%f",&tmpNum);
            m[i%4][i/4] = tmpNum;
        }
        this->model.push_back(m);
        this->view.push_back(v);
        glm::mat4 p = glm::perspective(glm::pi<float>() / 2, 1.f, 0.1f, 100.f);;
        this->projection.push_back(p);
#else
        glm:mat4 m,v,p;
        for(int i=0;i<16;i++)
        {
            scanf("%lf",&tmpNum);
            m[i/4][i%4] = tmpNum;
        }
        for(int i=0;i<16;i++)
        {
            scanf("%lf",&tmpNum);
            p[i/4][i%4] = tmpNum;
        }
        this->model.push_back(m);
        this->view.pish_back(v);
        this->projection.push_back(p);

#endif
    }
}
/*
void Fea::print(QString p_path)
{
    p_path.append(QString(".3df"));
    freopen(p_path.toStdString().c_str(),"w",stdout);

    for(int i=0;i<NUM;i++)
    {
        printf("%lf ",projectArea[i]);
        printf("%lf ",visSurfaceArea[i]);
        printf("%lf ",viewpointEntropy[i]);
        printf("%lf ",silhouetteLength[i]);
        printf("%lf ",silhouetteCurvature[i]);
        printf("%lf ",silhouetteCurvatureExtrema[i]);
        printf("%lf ",maxDepth[i]);
        printf("%lf ",depthDistribute[i]);
        printf("%lf ",meanCurvature[i]);
        printf("%lf ",gaussianCurvature[i]);
        printf("%e ",meshSaliency[i]);
        printf("%lf\n",abovePreference[i]);
    }

    fclose(stdout);
    freopen("CON","w",stdout);
}
*/

void Fea::printOut()
{
    freopen(output.toStdString().c_str(),"a+",stdout);
    printf("%s\n",fileName.at(t_case).toStdString().c_str());
    for(int i=0;i<12;i++)
    {
        if(i==10 || i == 8 || i == 9)
            printf("%lf ",log2(feaArray[i]));
        else
            printf("%lf ",feaArray[i]);
    }
    printf("\n");

    QString log = output;
    int pos = log.lastIndexOf('.');
    log.replace(pos,6,".log");
    freopen(log.toStdString().c_str(),"w",stdout);
    printf("%d\n",t_case+1);

    freopen("CON","a+",stdout);
    image.release();
}

void Fea::set_tCase()
{
    QString logFile = output;
    int pos = logFile.lastIndexOf('.');
    logFile.replace(pos,7,".log");
    qDebug() << "log file name " << logFile << endl;
    if(freopen(logFile.toStdString().c_str(),"r",stdin))
        scanf("%d",&t_case);
    else
        t_case = 0;
}

void Fea::showImage()
{
    render->showImage();
}


