#version 460

uniform vec3 ChunkPosition;
uniform float voxelSize;
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

layout (local_size_x = 16, local_size_y = 16,local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imgOutput;

layout (r32f, binding = 1) uniform image2D depthTex;

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
	vec3 uBoxMin = ChunkPosition;
	vec3 uBoxMax = ChunkPosition + vec3(voxelSize) * vec3(ChunkSize);
	
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

vec4 GetColorWithLighting(vec3 Color, vec3 Normal)
{
	const vec3 LightDir = normalize(vec3(1.0,1.0,0.5));
	float diffuse = max(dot(Normal,LightDir),0.0);
	vec3 litColor = Color * diffuse;

	vec3 Ambient = 0.2 * Color;
	litColor += Ambient;

	litColor = clamp(litColor,0,1);

	return vec4(litColor,1.0);
}

vec4 TraverseVoxelGrid(vec3 ro, vec3 rd,float eT, out float LastT)
{
	vec4 color = vec4(0.0,0.0,0.0,0.0);

	vec3 gridMax = ChunkPosition + vec3(voxelSize) * vec3(ChunkSize);

	const vec3 entry_pos = ((ro + rd * (eT)) - ChunkPosition) / vec3(voxelSize);
	ivec3 pos = ivec3(clamp(floor(entry_pos), vec3(0), vec3(ChunkSize-1)));

	ivec3 stepVector = ivec3(sign(rd));

	vec3 nextBoundary;
	for(int i = 0; i < 3; ++i)
	{
		if(stepVector[i] > 0)
			nextBoundary[i] = ChunkPosition[i] + (pos[i] + 1) * voxelSize;
		else
			nextBoundary[i] = ChunkPosition[i] + (pos[i]) * voxelSize;
	}

	vec3 tmax = (nextBoundary - ro) / rd;
	vec3 delta = vec3(voxelSize) / abs(rd);

	int lastDir = -1; // X - 0, Y - 2, Z - 4

	const int MAX_STEPS = 5000;
	float stepsT = 0.0;
	for (int steps = 0; steps < MAX_STEPS; ++steps)
	{
		if (getVoxel(ivec3(pos)))
		{
			color = getVoxelColor(ivec3(pos));

			
			vec3 normal = vec3(0.0);

			//When voxel is hit from side of chunk the normal is wrong
			if(steps == 0)
			{
				vec3 chunkMin = ChunkPosition;
				vec3 chunkMax = ChunkPosition + vec3(voxelSize) * vec3(ChunkSize);

				vec3 invDir = 1.0 / rd;
				vec3 t0s = (chunkMin - ro) * invDir;
				vec3 t1s = (chunkMax - ro) * invDir;

				vec3 tmin3 = min(t0s, t1s);
				vec3 tmax3 = max(t0s, t1s);

				float tEnter = max(max(tmin3.x, tmin3.y), tmin3.z);
				float tExit  = min(min(tmax3.x, tmax3.y), tmax3.z);

				vec3 eps = vec3(0.0001); // small epsilon to avoid precision issues
				vec3 entryPoint = ro + rd * tEnter;

				if(abs(entryPoint.x - ChunkPosition.x) < eps.x) normal = vec3(-1,0,0);
				else if(abs(entryPoint.x - chunkMax.x) < eps.x) normal = vec3(1,0,0);
				else if(abs(entryPoint.y - chunkMin.y) < eps.y) normal = vec3(0,-1,0);
				else if(abs(entryPoint.y - chunkMax.y) < eps.y) normal = vec3(0,1,0);
				else if(abs(entryPoint.z - chunkMin.z) < eps.z) normal = vec3(0,0,-1);
				else if(abs(entryPoint.z - chunkMax.z) < eps.z) normal = vec3(0,0,1);
			}
			else
			{
				if (lastDir == 0)       normal = vec3(-stepVector.x, 0.0, 0.0);
				else if (lastDir == 2)  normal = vec3(0.0, -stepVector.y, 0.0);
				else if (lastDir == 4)  normal = vec3(0.0, 0.0, -stepVector.z);
				else normal = vec3(0.0,0.0,0.0);
			}
			color = GetColorWithLighting(color.xyz,normal);
			break;
		}

		if(tmax.x < tmax.y)
		{
			if(tmax.x < tmax.z)
			{
				pos.x += stepVector.x;
				if(pos.x < 0 || pos.x >= ChunkSize.x) break;
				stepsT = tmax.x;
				tmax.x += delta.x;
				lastDir = 0;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= ChunkSize.z) break;
				stepsT = tmax.z;
				tmax.z += delta.z;
				lastDir = 4;
			}
		}
		else
		{
			if(tmax.y < tmax.z)
			{
				pos.y += stepVector.y;
				if (pos.y < 0 || pos.y >= ChunkSize.y) break;
				stepsT = tmax.y;
				tmax.y += delta.y;
				lastDir = 2;
			}
			else
			{
				pos.z += stepVector.z;
				if (pos.z < 0 || pos.z >= ChunkSize.z) break;
				stepsT = tmax.z;
				tmax.z += delta.z;
				lastDir = 4;
			}
		}
		LastT = eT;
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

	float CurrentPixelDepth;
	CurrentPixelDepth = imageLoad(depthTex,pixel).r;

	float hitT = 0.0;
	vec4 color = vec4(0);
	float LastT = -1.0;
	if(AABB(RayOrigin,RayDirection,hitT))
	{
		if(hitT >= CurrentPixelDepth)
			return;
		
		color = TraverseVoxelGrid(RayOrigin,RayDirection,hitT,LastT);
	}
	if(color != vec4(0))
		imageStore(imgOutput,pixel,color);

	if(LastT != -1.0 && color.a > 0.0)
	{
		imageStore(depthTex,pixel,vec4(LastT));
	}
}