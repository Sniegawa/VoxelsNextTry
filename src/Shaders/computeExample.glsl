#version 460

uniform vec3 gridPosition;
const vec3 voxelSize = vec3(0.25);
uniform ivec3 ChunkSize; // Size of dimentions of the voxel texture

struct Camera
{
	vec3 Pos;
	float pad1;
	vec3 Forward;
	float pad2;
	vec3 Right;
	float pad3;
	vec3 Up; // in theory i don't need this cause i can calculate it for now ill leave it TODO remove
	float aspect;
};

uniform ivec2 iResolution;

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

layout(binding = 0) uniform usampler3D voxelTex;

layout(std140, binding = 0) uniform CameraData {
	Camera camera;
};

vec4 colors[7] = vec4[7](
    vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.6, 0.3, 0.1, 1.0), //brown 1
    vec4(0.5, 0.2, 0.05, 1.0), //darker brown 2
    vec4(0.5, 0.5, 0.5, 1.0), // Gray 3
    vec4(0.4, 0.4, 0.4, 1.0), // Dark Gray 4
    vec4(0.2, 0.7, 0.2, 1.0), //Green 5
    vec4(0.15, 0.6, 0.15, 1.0) //Dark Green 6
);

bool getVoxel(ivec3 pos) {
    // Clamp to valid voxel coordinates
    pos = clamp(pos, ivec3(0), ChunkSize-1);
    return texelFetch(voxelTex, pos, 0).r != 0u;
}

vec4 getVoxelColor(ivec3 pos) {
    // Clamp to valid voxel coordinates
    pos = clamp(pos, ivec3(0), ChunkSize-1);
    return colors[texelFetch(voxelTex, pos, 0).r];
}

bool AABB(vec3 rayOrigin, vec3 rayDir, out float hitT)
{
	vec3 uBoxMin = gridPosition;
	vec3 uBoxMax = gridPosition + voxelSize * vec3(ChunkSize);
	
	if (rayOrigin.x > uBoxMin.x && rayOrigin.y > uBoxMin.y && rayOrigin.z > uBoxMin.z && rayOrigin.x < uBoxMax.x && rayOrigin.y < uBoxMax.y && rayOrigin.z < uBoxMax.z)
	{
		hitT = 0.0f;
		return true;
	}
	vec3 invDir = 1.0/rayDir;

	vec3 tMin = (uBoxMin - rayOrigin) * invDir;
	vec3 tMax = (uBoxMax - rayOrigin) * invDir;

	vec3 t1 = min(tMin,tMax);
	vec3 t2 = max(tMin,tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x,t2.y),t2.z);

	if(tNear < tFar && tFar > 0.0)
	{
		hitT = tNear;
				
		vec3 pos = rayOrigin + tNear * rayDir;
		return true;
	}
	hitT = 1e30f;
	return false;
}

vec4 TraverseVoxelGrid(vec3 ro, vec3 rd,float eT)
{
	vec4 color = vec4(0.0,0.0,0.0,1.0);

	vec3 gridMax = gridPosition + voxelSize * vec3(ChunkSize);

	const vec3 entry_pos = ((ro + rd * (eT + 0.0001f)) - gridPosition) / voxelSize;
	ivec3 pos = ivec3(clamp(floor(entry_pos), vec3(0), vec3(ChunkSize-1)));

	ivec3 stepVector = ivec3(sign(rd));

	vec3 nextBoundary;
	for(int i = 0; i < 3; ++i)
	{
		if(stepVector[i] > 0)
			nextBoundary[i] = gridPosition[i] + (pos[i] + 1) * voxelSize[i];
		else
			nextBoundary[i] = gridPosition[i] + (pos[i]) * voxelSize[i];
	}

	vec3 tmax = (nextBoundary - ro) / rd;
	vec3 delta = voxelSize / abs(rd);

	const int MAX_STEPS = 100000;
	for (int steps = 0; steps < MAX_STEPS; ++steps)
	{

		if (getVoxel(ivec3(pos)))
		{
			color = getVoxelColor(ivec3(pos));
			break;
		}

		if(tmax.x < tmax.y)
		{
			if(tmax.x < tmax.z)
			{
				pos.x += stepVector.x;
				if(pos.x < 0 || pos.x >= ChunkSize.x) break;
				tmax.x += delta.x;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= ChunkSize.z) break;
				tmax.z += delta.z;
			}
		}
		else
		{
			if(tmax.y < tmax.z)
			{
				pos.y += stepVector.y;
				if (pos.y < 0 || pos.y >= ChunkSize.y) break;
				tmax.y += delta.y;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= ChunkSize.z) break;
				tmax.z += delta.z;
			}
		}
	}

	return color;
}

void main()
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	if(any(greaterThanEqual(pixel, iResolution))) return;

	vec3 Forward = camera.Forward;
	vec3 Right = camera.Right;
	vec3 up = camera.Up;
	vec2 uv = (vec2(pixel) + 0.5) / (iResolution - 1.0);
	vec2 ndc = vec2(2*uv.x - 1,2 * uv.y - 1);
	float aspect = camera.aspect;
	ndc.x *= aspect;

	vec2 p = ndc * tan(radians(45.0f)/2);
	

	vec3 RayOrigin = camera.Pos;
	//vec3 RayDirection = normalize(Forward + uv.x * Right + uv.y * up);

	vec3 RayDirection = normalize(p.x * Right + p.y * up + Forward);

	float hitT = 0.0;
	vec4 color = vec4(0);
	
	if(AABB(RayOrigin,RayDirection,hitT))
	{
		color = TraverseVoxelGrid(RayOrigin,RayDirection,hitT);
		//color = VoxelTraverse(RayOrigin,RayDirection,hitT);
	}
	imageStore(imgOutput,pixel,color);
}