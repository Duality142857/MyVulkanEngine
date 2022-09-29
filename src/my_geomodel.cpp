#pragma once
#include"my_geomodel.h"

void GeoModel::bind(MyCommandBuffer& mcmdbuffer, uint32_t binding)
{
    VkDeviceSize offsets[]={0};
    mcmdbuffer.bindVertexBuffers(binding,1,&vertices.buffer,offsets);
    mcmdbuffer.bindIndexBuffer(indices.buffer,offsets[0]);
}

void GeoModel::draw(MyCommandBuffer& mcmdbuffer)
{

    mcmdbuffer.drawIndexed(indices.count,1);
    
}