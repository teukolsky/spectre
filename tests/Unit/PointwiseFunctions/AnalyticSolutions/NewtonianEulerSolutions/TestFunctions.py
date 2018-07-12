# Distributed under the MIT License.
# See LICENSE.txt for details.

import numpy as np


# Functions for testing IsentropicVortex.cpp
def mass_density(x, t, adiabatic_index, center, mean_velocity, strength):
    x_tilde = x - center - t * np.array(mean_velocity)
    temp = (1.0 - strength * strength * (adiabatic_index - 1.0) *
            np.exp(1.0 - np.dot(x_tilde[:2], x_tilde[:2])) /
            (8.0 * adiabatic_index * np.pi**2))
    return np.power(temp, 1.0 / (adiabatic_index - 1.0))


def velocity(x, t, adiabatic_index, center, mean_velocity, strength):
    x_tilde = x - center - t * np.array(mean_velocity)
    temp = (0.5 * strength *
            np.exp(0.5 * (1.0 - np.dot(x_tilde[:2], x_tilde[:2]))) / np.pi)
    velocity = np.copy(mean_velocity)
    velocity[0] -= x_tilde[1] * temp
    velocity[1] += x_tilde[0] * temp
    return velocity


def specific_internal_energy(x, t, adiabatic_index, center, mean_velocity,
                             strength):
    return (np.power(mass_density(x, t, adiabatic_index, center, mean_velocity,
                                  strength), adiabatic_index - 1.0) /
            (adiabatic_index - 1.0))


def momentum_density(x, t, adiabatic_index, center, mean_velocity, strength):
    return (mass_density(x, t, adiabatic_index, center, mean_velocity,
                         strength) *
            velocity(x, t, adiabatic_index, center, mean_velocity, strength))


def energy_density(x, t, adiabatic_index, center, mean_velocity, strength):
    veloc = velocity(x, t, adiabatic_index, center, mean_velocity, strength)
    return (mass_density(x, t,
                        adiabatic_index, center, mean_velocity, strength)
            * (0.5 * np.dot(veloc, veloc) +
               specific_internal_energy(x, t, adiabatic_index, center,
                                        mean_velocity, strength)))


# End functions for testing IsentropicVortex.cpp
