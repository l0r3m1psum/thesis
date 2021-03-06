#ifndef SIMULATOR_INCLUDE
#define SIMULATOR_INCLUDE

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
	float B; /* fuel quantity */
	bool N; /* fire presence */
	/* 24 bit padding */ /* NOTE: potrei inserire qui un altro booleano per indicare se uno dei vicini è in fiamme */
} state_t;
static_assert(sizeof (state_t) == 8, "bad size");

/* This struct contains only the parameter used during the simualtion, i.e.
 * already converted from the "row" form and "compressed" in the calculated
 * parameter S.
 */
typedef struct {
	uint16_t P; /* altimetry */
	/* 16bit padding */
	float S; /* inflammability percentage */
	float F; /* wind speed */
	float D; /* wind direction */
	float gamma; /* initial fuel */
} params_t;
static_assert(sizeof (params_t) == 20, "bad size");

typedef struct {
	state_t * restrict old_state;
	state_t * restrict new_state;
	params_t *params;
	uint64_t Wstar; /* number of cells along the x axis */
	uint64_t Lstar; /* number of cells along the y axis */
	uint64_t h; /* horizon */
	uint64_t s; /* snapshot frequency */
	uint32_t seed;
	float Delta;
	float tau; /* time step */
	float theta; /* ingnition threshold */
	float k0; /* optimization parameter */
	float k1; /* wind optimization */
	float k2; /* slope optimization */
	float L; /* length of the sides of the cells in meters */
} simulation_t;
static_assert(sizeof (simulation_t) == 88, "bad size");

#define pi 3.14159265359f

uint64_t
sim_index(uint64_t i, uint64_t j, simulation_t *s);

void
simulation_run(simulation_t *s, bool (*dump)(simulation_t *));

void
simulation_SIGINT_handler(int sig);

#endif /* SIMULATOR_INCLUDE */
