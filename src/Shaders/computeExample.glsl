#version 460


const vec3 gridMin = vec3(0);
const vec3 gridMax = vec3(4);

uniform int voxelGridSize;

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

bool getVoxel(ivec3 pos) {
    // Clamp to valid voxel coordinates
    pos = clamp(pos, ivec3(0), ivec3(voxelGridSize-1));
    return texelFetch(voxelTex, pos, 0).r != 0u;
}

vec4 colors[7] = vec4[7](
    vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.6, 0.3, 0.1, 1.0), //brown 1
    vec4(0.5, 0.2, 0.05, 1.0), //darker brown 2
    vec4(0.5, 0.5, 0.5, 1.0), // Gray 3
    vec4(0.4, 0.4, 0.4, 1.0), // Dark Gray 4
    vec4(0.2, 0.7, 0.2, 1.0), //Green 5
    vec4(0.15, 0.6, 0.15, 1.0) //Dark Green 6
);

vec4 getVoxelColor(ivec3 pos) {
    // Clamp to valid voxel coordinates
    pos = clamp(pos, ivec3(0), ivec3(voxelGridSize-1));
    return colors[texelFetch(voxelTex, pos, 0).r];
}

bool AABB(vec3 rayOrigin, vec3 rayDir, out float hitT)
{
	vec3 uBoxMin = gridMin;
	vec3 uBoxMax = gridMax;
	
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

vec4 TraverseVoxelGrid(vec3 RayOrigin, vec3 RayDirection,float entryT)
{
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	
	float dist = length(RayOrigin + RayDirection * entryT - RayOrigin);

	if (entryT == 1e30f) return color;

	const vec3 voxel_per_unit = vec3(voxelGridSize)/ (gridMax - gridMin);

	const vec3 entry_pos = ((RayOrigin + RayDirection * (entryT + 0.0001f)) - gridMin) * voxel_per_unit;
	vec3 pos = clamp(floor(entry_pos),0,float(voxelGridSize-1));


	vec3 rayDirVoxel = RayDirection * voxel_per_unit;
	vec3 stepVector = sign(rayDirVoxel);

	vec3 delta;
	vec3 tmax;
	for(int i=0;i<3;i++){
		if(rayDirVoxel[i] > 0.0){
			tmax[i] = (pos[i] + 1.0 - entry_pos[i]) / rayDirVoxel[i];
			delta[i] = 1.0 / rayDirVoxel[i];
		} else if(rayDirVoxel[i] < 0.0){
			tmax[i] = (pos[i] - entry_pos[i]) / rayDirVoxel[i];
			delta[i] = -1.0 / rayDirVoxel[i];
		} else {
			tmax[i] = 1e30;
			delta[i] = 1e30;
		}
	}


	const int MAX_STEPS = 100000;
	for (int steps = 0; steps < MAX_STEPS; ++steps)
	{

		if (getVoxel(ivec3(pos)))
		{

			// Distance along ray
			float hitDist = 0.0;

			// Compute distance from camera to voxel center
			vec3 voxelCenter = (pos + vec3(0.5)) / voxel_per_unit + gridMin;
			hitDist = length(voxelCenter - RayOrigin);

			// Scale color based on distance
			float intensity = 5.0 / (1.0 + hitDist); // prevents division by zero
			color = vec4(intensity, 0.0, 0.0, 1.0);
			color = getVoxelColor(ivec3(pos));
			break;
		}

		if(tmax.x < tmax.y)
		{
			if(tmax.x < tmax.z)
			{
				pos.x += stepVector.x;
				if(pos.x < 0 || pos.x >= voxelGridSize) break;
				tmax.x += delta.x;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= voxelGridSize) break;
				tmax.z += delta.z;
			}
		}
		else
		{
			if(tmax.y < tmax.z)
			{
				pos.y += stepVector.y;
				if (pos.y < 0 || pos.y >= voxelGridSize) break;
				tmax.y += delta.y;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= voxelGridSize) break;
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
	vec2 uv = (vec2(pixel) + 0.5) / vec2(iResolution) * 2.0 - 1.0;

	vec3 RayOrigin = camera.Pos;
	vec3 RayDirection = normalize(Forward + uv.x * Right + uv.y * up);

	float hitT = 0.0;
	vec4 color = vec4(0);
	AABB(RayOrigin,RayDirection,hitT);
	if(1 == 1)
	{
		color = TraverseVoxelGrid(RayOrigin,RayDirection,hitT);
	}
	imageStore(imgOutput,pixel,color);
}