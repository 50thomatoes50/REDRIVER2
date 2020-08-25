#include "THISDUST.H"

#include "LIBGTE.H"
#include "LIBGPU.H"
#include "INLINE_C.H"
#include "GTEREG.H"

#include "GAME/ASM/ASMTEST.H"
#include "GAME/C/CONVERT.H"
#include "GAME/C/CAMERA.H"
#include "GAME/C/DRAW.H"
#include "GAME/C/SYSTEM.H"
#include "GAME/C/PRES.H"
#include "GAME/C/SPOOL.H"
#include "GAME/C/CARS.H"
#include "GAME/C/PLAYERS.H"

int gDisplayDrawStats = 0;

struct LineDef_t
{
	VECTOR posA, posB;
	CVECTOR color;
};

LineDef_t gDebug_Lines[512];
int gDebug_numLines = 0;

void DrawDebugOverlays()
{
	VECTOR _zerov = { 0 };

	while(gDebug_numLines > 0)
	{
		LineDef_t& ld = gDebug_Lines[gDebug_numLines-1];

		gte_SetTransVector(&_zerov);
		gte_SetRotMatrix(&inv_camera_matrix);

		SVECTOR a, b;
		a.vx = ld.posA.vx - camera_position.vx;
		a.vy = ld.posA.vy - camera_position.vy;
		a.vz = ld.posA.vz - camera_position.vz;

		b.vx = ld.posB.vx - camera_position.vx;
		b.vy = ld.posB.vy - camera_position.vy;
		b.vz = ld.posB.vz - camera_position.vz;

		gte_ldv3(&a, &b, &b);

		gte_rtpt();
		gte_avsz4();
		
		int z;
		gte_stopz(&z);

		if (z > 0)
		{
			LINE_F2* line = (LINE_F2*)current->primptr;
			setLineF2(line);
			setSemiTrans(line, 1);

			gte_stsxy0(&line->x0)
			gte_stsxy1(&line->x1);

			line->r0 = ld.color.r;
			line->g0 = ld.color.g;
			line->b0 = ld.color.b;

			addPrim(current->ot + 2, line);

			current->primptr += sizeof(LINE_F2);
		}
		gDebug_numLines--;
	}
	gDebug_numLines = 0;

	DR_TPAGE* tp = (DR_TPAGE*)current->primptr;
	setDrawTPage(tp, 0, 0, 0);
	addPrim(current->ot + 2, tp);
	current->primptr += sizeof(DR_TPAGE);

	char tempBuf[128];

	int primTabLeft = current->primptr - current->primtab;

	if (gDisplayDrawStats)
	{
		sprintf(tempBuf, "Primtab: %d of %d", primTabLeft, 0x1a180);
		PrintString(tempBuf, 10, 80);

		extern volatile int spoolactive;
		sprintf(tempBuf, "Spooling: %d spec: %d, active: %d", doSpooling, allowSpecSpooling, spoolactive);
		PrintString(tempBuf, 10, 90);

		extern int numActiveCops;
		extern int numCopCars;
		extern int numCivCars;
		extern int numParkedCars;
		extern int maxCopCars;
		extern int maxCivCars;

		sprintf(tempBuf, "Civs: %d - %d parked - max %d", numCivCars, numParkedCars, maxCivCars);
		PrintString(tempBuf, 10, 100);

		sprintf(tempBuf, "Cops: %d - %d active - max %d", numCopCars, numActiveCops, maxCopCars);
		PrintString(tempBuf, 10, 110);

		
		int playerCar = player[0].playerCarId;
		int speed = car_data[playerCar].hd.speed;

		sprintf(tempBuf, "Speed: %d", speed);
		PrintString(tempBuf, 10, 120);
	}
}

void Debug_AddLine(VECTOR& pointA, VECTOR& pointB, CVECTOR& color)
{
	if (gDebug_numLines + 1 > 512)
		return;

	int dx = camera_position.vx - pointA.vx;
	int dz = camera_position.vz - pointA.vz;

	if (dx * dx + dz * dz > (15000*15000))
		return;

	LineDef_t& ld = gDebug_Lines[gDebug_numLines++];
	ld.posA = pointA;
	ld.posB = pointB;
	ld.color = color;

	ld.posA.vy *= -1;
	ld.posB.vy *= -1;
}

void Debug_AddLineOfs(VECTOR& pointA, VECTOR& pointB, VECTOR& ofs, CVECTOR& color)
{
	if (gDebug_numLines + 1 > 512)
		return;

	int dx = camera_position.vx - ofs.vx;
	int dz = camera_position.vz - ofs.vz;

	if (dx * dx + dz * dz > (15000 * 15000))
		return;

	LineDef_t& ld = gDebug_Lines[gDebug_numLines++];
	ld.posA = pointA;
	ld.posB = pointB;
	ld.color = color;

	ld.posA.vx += ofs.vx;
	ld.posA.vy += ofs.vy;
	ld.posA.vz += ofs.vz;

	ld.posB.vx += ofs.vx;
	ld.posB.vy += ofs.vy;
	ld.posB.vz += ofs.vz;

	ld.posA.vy *= -1;
	ld.posB.vy *= -1;
}

int g_FreeCameraControls = 0;
int g_FreeCameraEnabled = 0;
VECTOR g_FreeCameraPosition;
SVECTOR g_FreeCameraRotation;
VECTOR g_FreeCameraVelocity;
extern int cursorX, cursorY, cursorOldX, cursorOldY;

extern VECTOR lis_pos;

void DoFreeCamera()
{
	if (g_FreeCameraEnabled == 0)
	{
		g_FreeCameraPosition = camera_position;
		g_FreeCameraRotation = camera_angle;
		g_FreeCameraVelocity.vx = 0;
		g_FreeCameraVelocity.vy = 0;
		g_FreeCameraVelocity.vz = 0;
		return;
	}

	camera_position = g_FreeCameraPosition;
	camera_angle = g_FreeCameraRotation;
	lis_pos = camera_position;
	
	BuildWorldMatrix();

	g_FreeCameraPosition.vx += FIXED(g_FreeCameraVelocity.vx);
	g_FreeCameraPosition.vy += FIXED(g_FreeCameraVelocity.vy);
	g_FreeCameraPosition.vz += FIXED(g_FreeCameraVelocity.vz);

	// deaccel
	g_FreeCameraVelocity.vx -= (g_FreeCameraVelocity.vx / 4);
	g_FreeCameraVelocity.vy -= (g_FreeCameraVelocity.vy / 4);
	g_FreeCameraVelocity.vz -= (g_FreeCameraVelocity.vz / 4);

	// accel
	if ((g_FreeCameraControls & 0x1) || (g_FreeCameraControls & 0x2)) // forward/back
	{
		int sign = (g_FreeCameraControls & 0x2) ? -1 : 1;

		g_FreeCameraVelocity.vx += (inv_camera_matrix.m[2][0] * 32) * sign;
		g_FreeCameraVelocity.vy += (inv_camera_matrix.m[2][1] * 32) * sign;
		g_FreeCameraVelocity.vz += (inv_camera_matrix.m[2][2] * 32) * sign;
	}

	// side
	if ((g_FreeCameraControls & 0x4) || (g_FreeCameraControls & 0x8)) // right/left
	{
		int sign = (g_FreeCameraControls & 0x8) ? 1 : -1;

		g_FreeCameraVelocity.vx += (inv_camera_matrix.m[0][0] * 32) * sign;
		g_FreeCameraVelocity.vy += (inv_camera_matrix.m[0][1] * 32) * sign;
		g_FreeCameraVelocity.vz += (inv_camera_matrix.m[0][2] * 32) * sign;
	}


}