#include"my_model.h"
#include<unordered_map>
#include<fstream>
void MyExtModel::loadModel(const std::string& path)
{
    Assimp::Importer importer;

    std::vector<MyVertex_Default> vbuffer;
    std::vector<uint32_t> ibuffer;
    std::unordered_map<MyVertex_Default, uint32_t> uniqueVertices{};

    const aiScene* scene=importer.ReadFile(path,
    aiProcess_Triangulate 
    | aiProcess_GenSmoothNormals 
    | aiProcess_FlipUVs 
    | aiProcess_CalcTangentSpace
    | aiProcess_SortByPType);
    // | aiProcess_JoinIdenticalVertices);
    // | aiProcess_SortByPType);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout<<importer.GetErrorString()<<std::endl;
    }

    processNode(scene->mRootNode,scene, vbuffer, ibuffer,uniqueVertices);

    std::cout<<"vertexNum: "<<vbuffer.size()<<std::endl;
    std::cout<<"indexNum: "<<ibuffer.size()<<std::endl;

    center/=vbuffer.size();
    std::cout<<"center: "<<center<<std::endl;
    std::cout<<"boundingMin: "<<boundingMin<<std::endl;
    std::cout<<"boundingMax: "<<boundingMax<<std::endl;
    float size_reci=1.f/(boundingMax-boundingMin).norm();
    
    auto y=boundingMin.y;
    y-=center.y;
    y*=4*size_reci;


    for(auto& v: vbuffer)
    {
        v.position-=center;
        v.position*=4*size_reci;
        v.position.y-=y;
    }

    rhi->createDataBuffer(vbuffer.data(),sizeof(vbuffer[0])*vbuffer.size(),vertices.buffer,vertices.memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    rhi->createDataBuffer(ibuffer.data(),sizeof(ibuffer[0])*ibuffer.size(),indices.buffer,indices.memory,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    
    vertices.count=vbuffer.size();
    indices.count=ibuffer.size();
    for(auto& name:textureSet)
    {
        std::cout<<name<<std::endl;
    }

    std::cout<<"numNodes: "<<numNodes<<std::endl;
    std::cout<<"numMeshes: "<<currentLoadedMeshNum<<std::endl;
    std::cout<<"numTextures: "<<textureSet.size()<<std::endl;

    // for(const auto& v: vbuffer)
    // {
    //     rigidBody.positions.push_back(v.position);
    // }
    
    // rigidBody.init();
    // rigidBody.position=(modelMat*MyGeo::Vec4f{rigidBody.position,1.f}).head;

    // std::cout<<"position: "<<rigidBody.position<<std::endl;

}

void MyExtModel::processNode(aiNode* node, const aiScene* scene,std::vector<MyVertex_Default>& vbuffer, std::vector<uint32_t>& ibuffer,std::unordered_map<MyVertex_Default, uint32_t>& uniqueVertices)
{
    ++numNodes;
    NodeInfo nodeInfo;

    for(uint32_t i=0;i<node->mNumMeshes;++i)
    {
        MeshInfo meshInfo;
        aiMesh* mesh=scene->mMeshes[node->mMeshes[i]];
        meshInfo.firstIndex=firstIndex;
        processMesh(mesh,scene, vbuffer, ibuffer,uniqueVertices);
        meshInfo.count=ibuffer.size()-firstIndex;
        firstIndex=ibuffer.size();
        nodeInfo.meshInfos.push_back(meshInfo);
    }
    nodeInfos.push_back(nodeInfo);

    for(uint32_t i=0;i<node->mNumChildren;++i)
    {
        processNode(node->mChildren[i], scene, vbuffer, ibuffer,uniqueVertices);
    }
}

void MyExtModel::processMesh(const aiMesh* mesh, const aiScene* scene,std::vector<MyVertex_Default>& vbuffer, std::vector<uint32_t>& ibuffer,std::unordered_map<MyVertex_Default, uint32_t>& uniqueVertices)
{
    ++currentLoadedMeshNum;
    auto meshMatIndex=mesh->mMaterialIndex;
    auto mat=scene->mMaterials[meshMatIndex];
    std::string textureName;
    loadMatTextures(mat, aiTextureType_DIFFUSE, textureName);
    std::cout<<"textureName "<<textureName<<std::endl;
    bool flag=true;
    auto it=texturePool->indexMap.find(textureName);
    if(it==texturePool->indexMap.end()) flag=false;

    auto modelParentPath=std::filesystem::path(modelPathStr).parent_path();

    auto texturePath= modelParentPath/std::filesystem::path(textureName);
    std::cout<<"texturepath: "<<texturePath.c_str()<<std::endl;
    if(it==texturePool->indexMap.end() && textureName.size()!=0)
    {
        flag=true;
        texturePool->addTexture(texturePath.c_str());
    }

    auto textureId=flag?texturePool->indexMap[textureName]:-1;
    std::cout<<"tid "<<textureId<<std::endl;
    std::cout<<"numVertices mesh: "<<mesh->mNumVertices<<std::endl;
    //顶点，normal, 纹理坐标
    for(int i=0;i<mesh->mNumVertices;++i)
    {
        MyVertex_Default v;
        v.position={mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z};

        v.normal={mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z};

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            v.texCoord={mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y,(float)textureId};
        }
        else 
        {
            v.texCoord.z=(float)textureId;
        }

        if(uniqueVertices.count(v)==0)
        {
            uniqueVertices[v]=vbuffer.size();
            vbuffer.push_back(v);
            center+=v.position;
            if(vbuffer.size()==1)
            {
                boundingMax=boundingMin=v.position;
            }
            if(v.position.x < boundingMin.x) boundingMin.x=v.position.x;
            if(v.position.y < boundingMin.y) boundingMin.y=v.position.y;
            if(v.position.z < boundingMin.z) boundingMin.z=v.position.z;
            if(v.position.x > boundingMax.x) boundingMax.x=v.position.x;
            if(v.position.y > boundingMax.y) boundingMax.y=v.position.y;
            if(v.position.z > boundingMax.z) boundingMax.z=v.position.z;
        }
        ibuffer.push_back(uniqueVertices[v]);
    }
    //
    // std::cout<<"faces: "<<mesh->mNumFaces<<std::endl;
    // for(int i=0;i<mesh->mNumFaces;++i)
    // {
    //     aiFace face=mesh->mFaces[i];
    //     for(uint32_t j=0;j<face.mNumIndices;++j)
    //     {
    //         ibuffer.push_back(face.mIndices[j]);
    //     }
    // }

        // loadMatTextures(mat, aiTextureType_SPECULAR);
        // loadMatTextures(mat, aiTextureType_AMBIENT);
        // loadMatTextures(mat, aiTextureType_EMISSIVE);
        // loadMatTextures(mat, aiTextureType_REFLECTION);
        // loadMatTextures(mat, aiTextureType_HEIGHT);
        // loadMatTextures(mat, aiTextureType_SHININESS);
    std::cout<<"finish one mesh"<<std::endl;
}

void MyExtModel::loadMatTextures(aiMaterial* mat, aiTextureType type, std::string& firstTextureName)
{
    auto texCount=mat->GetTextureCount(type);
    for(auto i=0;i!=texCount;++i)
    {
        aiString str;
        mat->GetTexture(type,0,&str);
        if(i==0) firstTextureName=str.C_Str();
        textureSet.insert(str.C_Str());
    }
}

