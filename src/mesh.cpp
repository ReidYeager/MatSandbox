
#include "src/application.h"

#include "src/common.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <stdio.h>
#include <unordered_map>

size_t HashInt(int32_t value)
{
  value += (0x9e3779b9 * value) >> 3;
  return value;
}

// Used to map vertices into an unordered array during mesh building
namespace std {
template<> struct hash<MsVertex>
{
  size_t operator()(MsVertex const& vertex) const
  {
    // Keep all bits, don't round
    Vec3I* iPos = (Vec3I*)&vertex.position;
    Vec3I* iNor = (Vec3I*)&vertex.normal;
    Vec2I* iUv  = (Vec2I*)&vertex.uv;

    size_t posHash = 0, normHash = 0, uvHash = 0;

    posHash ^= HashInt(iPos->x);
    posHash ^= HashInt(iPos->y);
    posHash ^= HashInt(iPos->z);

    normHash ^= HashInt(iNor->x);
    normHash ^= HashInt(iNor->y);
    normHash ^= HashInt(iNor->z);

    uvHash ^= HashInt(iUv->x);
    uvHash ^= HashInt(iUv->y);

    return posHash ^ normHash ^ uvHash;
  }
};
}

MsbResult LoadTinyObjMesh(const char* path, tinyobj::attrib_t* attrib, std::vector<tinyobj::shape_t>* shapes)
{
  std::string loadWarnings, loadErrors;
  std::vector<tinyobj::material_t> materials;


  if (!tinyobj::LoadObj(attrib, shapes, &materials, &loadWarnings, &loadErrors, path))
  {
    printf("Failed to load the mesh \"%s\"\n    > Tinyobj warnings : %s\n    > Tinyobj errors : %s",
      path,
      loadWarnings.c_str(),
      loadErrors.c_str());
    return Msb_Fail_External;
  }

  return Msb_Success;
}

MsbResult BuildMeshFromFile(const char* path, MsMeshInfo* outMeshInfo)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  MSB_ATTEMPT(LoadTinyObjMesh(path, &attrib, &shapes));

  // Assemble mesh =====
  std::unordered_map<MsVertex, uint32_t> vertMap = {};
  MsVertex vert{};
  for (const auto& shape : shapes)
  {
    for (const auto& indexSet : shape.mesh.indices)
    {
      vert.position = {
        attrib.vertices[3 * indexSet.vertex_index + 0],
        attrib.vertices[3 * indexSet.vertex_index + 1],
        attrib.vertices[3 * indexSet.vertex_index + 2]
      };

      vert.uv = {
        attrib.texcoords[2 * indexSet.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * indexSet.texcoord_index + 1],
      };

      vert.normal = {
        attrib.normals[3 * indexSet.normal_index + 0],
        attrib.normals[3 * indexSet.normal_index + 1],
        attrib.normals[3 * indexSet.normal_index + 2]
      };

      if (vertMap.count(vert) == 0)
      {
        vertMap[vert] = (uint32_t)outMeshInfo->verts.size();

        MsVertex properVert{};
        properVert.position = { vert.position.x, vert.position.y, vert.position.z };
        properVert.uv = { vert.uv.x, vert.uv.y };
        properVert.normal = { vert.normal.x, vert.normal.y, vert.normal.z };

        outMeshInfo->verts.push_back(properVert);
      }
      outMeshInfo->indices.push_back(vertMap[vert]);
    }
  }

  return Msb_Success;
}

MsbResult LoadMesh(const char* path, OpalMesh* outMesh)
{
  MsMeshInfo meshData = {};
  MSB_ATTEMPT(BuildMeshFromFile(path, &meshData));

  OpalMeshInitInfo meshInfo = {};
  meshInfo.vertexCount = meshData.verts.size();
  meshInfo.pVertices = meshData.verts.data();
  meshInfo.indexCount = meshData.indices.size();
  meshInfo.pIndices = meshData.indices.data();
  MSB_ATTEMPT_OPAL(OpalMeshInit(outMesh, meshInfo));

  return Msb_Success;
}

MsbResult MsbApplication::LoadMesh(char* path)
{
  MsMeshInfo meshData = {};
  MSB_ATTEMPT(BuildMeshFromFile(path, &meshData));

  OpalMeshInitInfo meshInfo = {};
  meshInfo.vertexCount = meshData.verts.size();
  meshInfo.pVertices = meshData.verts.data();
  meshInfo.indexCount = meshData.indices.size();
  meshInfo.pIndices = meshData.indices.data();
  OpalMesh newMesh;
  MSB_ATTEMPT_OPAL(OpalMeshInit(&newMesh, meshInfo));

  m_meshes.push_back(newMesh);

  return Msb_Success;
}
