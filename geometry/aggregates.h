#pragma once
#include"primitives.h"
#include<vector>
#include<array>

// int leafNum=0;

struct Aggreate: public Object
{
    
};


struct BVH_Node
{
    BB3f bound;
    std::array<BVH_Node*,2> children;
    int splitAxis;
    int primOffset, numPrim;
    BVH_Node(int primOffset, int numPrim, const BB3f& box): primOffset{primOffset},numPrim{numPrim},bound{bound},children{nullptr,nullptr}
    {}

    BVH_Node(int splitAxis, BVH_Node* c0, BVH_Node* c1):children{c0,c1},bound{Union(c0->bound,c1->bound)},splitAxis{splitAxis},numPrim{0}
    {}
};

struct BVH_LinearNode
{
    BB3f bound;
    union 
    {
        int primOffset;//leaf
        int rightChildIndex;//interior
    };
    uint16_t numPrim;//0 for interior node, or primitive number for leaf
    uint8_t axis; // x y or z for interior node
    uint8_t pad[1];
};


struct BVHPrimitiveInfo
{
    int primId;
    BB3f bound;
    Point centroid;

    BVHPrimitiveInfo(int primId, const BB3f& bound):primId{primId},bound{bound},centroid{0.5f*bound.min+0.5f*bound.max}{}
};

struct BVH : public Aggreate
{
    const int maxPrimsInNode=1;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<BVH_LinearNode> linearNodes;

    BVH(std::vector<std::shared_ptr<Primitive>> p): primitives{std::move(p)}
    {
        if(primitives.size()==0) return;
        std::vector<BVHPrimitiveInfo> primInfos;
        primInfos.reserve(primitives.size());
        for(int i=0;i<primitives.size();++i)
        {
            primInfos.emplace_back(BVHPrimitiveInfo{i,primitives[i]->bound()});
        }
        
        int nodeNum=0;
        std::vector<std::shared_ptr<Primitive>> orderedPrims;
        BVH_Node* root;
        root=recursiveBuild(primInfos,0,primitives.size(),nodeNum,orderedPrims);
        primitives.swap(orderedPrims);

        //compute representation of depth first traversal of bvh tree
        linearNodes.resize(nodeNum);
        int offset=0;
        flattenTree(root,offset);
    }

    BVH_Node* recursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfos, int start, int end, int& nodeNum, std::vector<std::shared_ptr<Primitive>>& orderedPrims)
    {
        BVH_Node* node;//=new BVH_Node();
        nodeNum++;
        BB3f bound;
        for(int i=start;i<end;++i)
        {
            bound=Union(bound,primitiveInfos[i].bound);
        }
        int numPrim=end-start;
        if(numPrim==1)
        {
            //leaf
            int first=orderedPrims.size();

            int primId=primitiveInfos[start].primId;
            orderedPrims.emplace_back(primitives[primId]);

            node=new BVH_Node(first,numPrim,bound);
        }
        else 
        {
            BB3f centroidsBound{primitiveInfos[start].centroid.v3};
            for(int i=start+1;i<end;++i)
            {
                centroidsBound=Union(centroidsBound,primitiveInfos[i].centroid.v3);
            }
            int axis=centroidsBound.mainAxis();
            int mid=(start+end)/2;
            //all centroid points clapse to one point, stop recursion and create a leaf
            if(centroidsBound.max[axis]==centroidsBound.min[axis])
            {
                int first=orderedPrims.size();
                for(int i=start;i<end;++i)
                {
                    int primId=primitiveInfos[i].primId;
                    orderedPrims.emplace_back(primitives[primId]);
                }
                node=new BVH_Node(first,numPrim,bound);
            }
            else 
            {
                //split

                if(numPrim<=4)
                {
                    //partial sort so that nth is replaced with median value, and before that are smaller, after are larger
                    std::nth_element(&primitiveInfos[start],&primitiveInfos[mid],&primitiveInfos[end-1]+1,[axis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b){return a.centroid.v3[axis]<b.centroid.v3[axis];});
                }
                else 
                {
                    constexpr int numBuckets=12;
                    struct BucketInfo
                    {
                        int count=0;
                        BB3f bound;
                    };
                    BucketInfo buckets[numBuckets];
                    
                    for (int i = start; i < end; ++i)
                    {
                        //b is the bucket id of current primitive
                        int b=numBuckets*centroidsBound.offset(primitiveInfos[i].centroid.v3)[axis];
                        if(b==numBuckets) --b;
                        buckets[b].count++;
                        buckets[b].bound=Union(buckets[b].bound,primitiveInfos[i].bound);
                    }
                    
                    float cost[numBuckets-1];
                    for(int i=0;i<numBuckets-1;++i)
                    {
                        BB3f b0,b1;
                        int count0=0,count1=0;
                        for(int j=0;j<=i;++j)
                        {
                            b0=Union(b0,buckets[j].bound);
                            count0+=buckets[j].count;
                        }
                        for(int j=i+1;j<numBuckets;++j)
                        {
                            b1=Union(b1,buckets[j].bound);
                            count1+=buckets[j].count;
                        }
                        cost[i]=0.125f+(count0*b0.area()+count1*b1.area())/bound.area();
                    }
                    //find split position
                    float minCost=cost[0];
                    int splitBucketId=0;
                    for(int i=1;i<numBuckets-1;++i)
                    {
                        if(cost[i]<minCost)
                        {
                            minCost=cost[i];
                            splitBucketId=i;
                        }
                    }
                    
                    float leafCost=numPrim;
                    if(numPrim>maxPrimsInNode || minCost<leafCost)
                    {
                        BVHPrimitiveInfo* pmid=std::partition(&primitiveInfos[start],&primitiveInfos[end-1]+1,[=](const BVHPrimitiveInfo& pi)
                        {
                            int b=numBuckets*centroidsBound.offset(pi.centroid.v3)[axis];
                            if(b==numBuckets) b=numBuckets-1;
                            return b<=splitBucketId;
                        });
                        mid=pmid-&primitiveInfos[0];
                    }
                    else 
                    {
                        int first=orderedPrims.size();
                        for(int i=start;i<end;++i)
                        {
                            int primId=primitiveInfos[i].primId;
                            orderedPrims.emplace_back(primitives[primId]);
                        }
                        node=new BVH_Node(first,numPrim,bound);
                        return node;
                    }
                }

                node=new BVH_Node(axis,recursiveBuild(primitiveInfos,start,mid,nodeNum,orderedPrims),recursiveBuild(primitiveInfos,mid,end,nodeNum,orderedPrims));
            }
        }
        return node;
    }


    int flattenTree(BVH_Node* node, int& offset)
    {
        BVH_LinearNode& linearNode=linearNodes[offset];
        linearNode.bound=node->bound;
        int t=offset++;
        if(node->numPrim>0)
        {
            linearNode.primOffset=node->primOffset;
            linearNode.numPrim=node->numPrim;
        }
        else 
        {
            linearNode.axis=node->splitAxis;
            linearNode.numPrim=0;
            flattenTree(node->children[0],offset);
            linearNode.rightChildIndex=flattenTree(node->children[1],offset);
        }
        return t;
    }

    virtual BB3f bound() const 
    {
        return linearNodes[0].bound;
    }

    virtual bool hit(const Ray& ray, HitRecord& rec) const 
    {
        bool hitflag=false;
        MyGeo::Vec3f recidir{1.f/ray.direction.x,1.f/ray.direction.y,1.f/ray.direction.z};
        bool dirNeg[3]={recidir.x<0,recidir.y<0,recidir.z<0};
        int toVisitOffset=0;
        int currentNodeIndex=0;
        int nodesToVisit[64];
        while(true)
        {
            const BVH_LinearNode& node=linearNodes[currentNodeIndex];
            if(node.bound.hitP(ray))
            {
                if(node.numPrim>0)//hit leaf
                {
                    for(int i=0;i<node.numPrim;++i)
                    {
                        if(primitives[node.primOffset+i]->hit(ray,rec)) hitflag=true;
                    }
                    if(toVisitOffset==0) break;
                    currentNodeIndex=nodesToVisit[--toVisitOffset];
                }
                else 
                {
                    //add child with smaller t on the ray to the stack first to increase efficiency!
                    if(dirNeg[node.axis])
                    {
                        nodesToVisit[toVisitOffset++]=currentNodeIndex+1;
                        currentNodeIndex=node.rightChildIndex;
                    }
                    else 
                    {
                        nodesToVisit[toVisitOffset++]=node.rightChildIndex;
                        ++currentNodeIndex;
                    }
                }

            }
            else 
            {
                if(toVisitOffset==0) break;
                currentNodeIndex=nodesToVisit[--toVisitOffset];
            }
        }
        return hitflag;

    }

};

