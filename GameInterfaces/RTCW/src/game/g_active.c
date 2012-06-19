
#include "g_local.h"
#include  "g_rtcwbot_interface.h"

extern void G_CheckForCursorHints( gentity_t *ent );


void PushBot( gentity_t *ent, gentity_t *other ) {
	vec3_t dir, ang, f, r;
	float oldspeed;

	// dont push when scripted not to be pushed
	if ( !other->client || !other->client->sess.botPush ) {
		return;
	}

	oldspeed = VectorLength( other->client->ps.velocity );
	if ( oldspeed < 200 ) {
		oldspeed = 200;
	}
	//
	VectorSubtract( other->r.currentOrigin, ent->r.currentOrigin, dir );
	VectorNormalize( dir );
	vectoangles( dir, ang );
	AngleVectors( ang, f, r, NULL );
	f[2] = 0;
	r[2] = 0;
	//
	VectorMA( other->client->ps.velocity, 200, f, other->client->ps.velocity );
	VectorMA( other->client->ps.velocity, 100 * ( ( level.time + ( ent->s.number * 1000 ) ) % 4000 < 2000 ? 1.0 : -1.0 ), r, other->client->ps.velocity );
	//
	if ( VectorLengthSquared( other->client->ps.velocity ) > SQR( oldspeed ) ) {
		VectorNormalize( other->client->ps.velocity );
		VectorScale( other->client->ps.velocity, oldspeed, other->client->ps.velocity );
	}
}
/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t   *client;
	float count;
	vec3_t angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;     // didn't take any damage
	}

	if ( count > 127 ) {
		count = 127;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
		client->ps.damageYaw = angles[YAW] / 360.0 * 256;
	}

	// play an apropriate pain sound
	if ( ( level.time > player->pain_debounce_time ) && !( player->flags & FL_GODMODE ) && !( player->r.svFlags & SVF_BOT ) && !( player->s.powerups & PW_INVULNERABLE ) ) {    //----(SA)
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
	}

	client->ps.damageEvent++;   // Ridah, always increment this since we do multiple view damage anims

	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}


#define MIN_BURN_INTERVAL 399 // JPW NERVE set burn timeinterval so we can do more precise damage (was 199 old model)

/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	int waterlevel;

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;   // don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time ) {

			if ( ent->client->ps.powerups[PW_BREATHER] ) { // take air from the breather now that we need it
				ent->client->ps.powerups[PW_BREATHER] -= ( level.time - ent->client->airOutTime );
				ent->client->airOutTime = level.time + ( level.time - ent->client->airOutTime );
			} else {
				// drown!
				ent->client->airOutTime += 1000;
				if ( ent->health > 0 ) {
					// take more damage the longer underwater
					ent->damage += 2;
					if ( ent->damage > 15 ) {
						ent->damage = 15;
					}

					Bot_Event_Drowning( ent-g_entities );

					// play a gurp sound instead of a normal pain sound
					if ( ent->health <= ent->damage ) {
						G_Sound( ent, G_SoundIndex( "*drown.wav" ) );
					} else if ( rand() & 1 ) {
						G_Sound( ent, G_SoundIndex( "sound/player/gurp1.wav" ) );
					} else {
						G_Sound( ent, G_SoundIndex( "sound/player/gurp2.wav" ) );
					}

					// don't play a normal pain sound
					ent->pain_debounce_time = level.time + 200;

					G_Damage( ent, NULL, NULL, NULL, NULL,
							  ent->damage, DAMAGE_NO_ARMOR, MOD_WATER );
				}
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if ( waterlevel && ( ent->watertype & CONTENTS_LAVA ) ) {
		if ( ent->health > 0 && ent->pain_debounce_time <= level.time ) {
			if ( ent->watertype & CONTENTS_LAVA ) {
				G_Damage( ent, NULL, NULL, NULL, NULL,
						  30 * waterlevel, 0, MOD_LAVA );
			}
		}
	}

	//
	// check for burning from flamethrower
	//
	// JPW NERVE MP way
	if ( ent->s.onFireEnd && ent->client ) {
		if ( level.time - ent->client->lastBurnTime >= MIN_BURN_INTERVAL ) {

			// JPW NERVE server-side incremental damage routine / player damage/health is int (not float)
			// so I can't allocate 1.5 points per server tick, and 1 is too weak and 2 is too strong.
			// solution: allocate damage far less often (MIN_BURN_INTERVAL often) and do more damage.
			// That way minimum resolution (1 point) damage changes become less critical.

			ent->client->lastBurnTime = level.time;
			if ( ( ent->s.onFireEnd > level.time ) && ( ent->health > 0 ) ) {
				gentity_t *attacker;
				attacker = g_entities + ent->flameBurnEnt;
				G_Damage( ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER ); // JPW NERVE was 7
			}
		}
	}
	// jpw


	// poisoned
	if ( ent->poisoned && ent->client ) {
		if ( ( level.time >= ( ent->lastPoisonTime + 1000 ) ) && ( ent->health > 0 ) ) {
			int n = rand() % 3;
			gentity_t *attacker = g_entities + ent->poisonEnt;
			vec3_t angles;

			if ( n == 0 ) {
				G_Sound( ent, G_SoundIndex( "sound/player/bj2/death1.wav" ) );
			} else if ( n == 1 ) {
				G_Sound( ent, G_SoundIndex( "sound/player/bj2/death2.wav" ) );
			} else if ( n == 2 ) {
				G_Sound( ent, G_SoundIndex( "sound/player/bj2/death3.wav" ) );
			}

			angles[YAW] = ( crandom() * 90 );
			angles[PITCH] = ( crandom() * 90 );
			angles[ROLL] = 0;
			SetClientViewAngle( ent, angles );

			G_Damage( ent, attacker, attacker, NULL, NULL, g_poisonDmg.integer, 0, MOD_POISON );
			ent->lastPoisonTime = level.time;
		}
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if ( ent->r.svFlags & SVF_BOT ) {
		return;
	}

	if ( ent->waterlevel && ( ent->watertype & CONTENTS_LAVA ) ) { //----(SA)	modified since slime is no longer deadly
		ent->s.loopSound = level.snd_fry;
	} else {
		ent->s.loopSound = 0;
	}
}


//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int i, j;
	trace_t trace;
	gentity_t   *other;

	memset( &trace, 0, sizeof( trace ) );
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		for ( j = 0 ; j < i ; j++ ) {
			if ( pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if ( j != i ) {
			continue;   // duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		// cs: modified so they push inv humans too (for annoying revives)
		if ( ( ent->client ) &&
			 ( ( other->r.svFlags & SVF_BOT ) || ( other->client && other->client->ps.powerups[PW_INVULNERABLE] ) ) ) {
			PushBot( ent, other );
		}

		// if we are standing on their head, then we should be pushed also
		if ( ( ent->r.svFlags & SVF_BOT || ( other->client && other->client->ps.powerups[PW_INVULNERABLE] ) ) &&
			 ( ent->s.groundEntityNum == other->s.number && other->client ) ) {
			PushBot( other, ent );
		}
		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}
}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void    G_TouchTriggers( gentity_t *ent ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	trace_t trace;
	vec3_t mins, maxs;
	static vec3_t range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER ) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			// MrE: always use capsule for player
			//if ( !trap_EntityContactCapsule( mins, maxs, hit ) ) {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof( trace ) );

		if ( hit->touch ) {
			hit->touch( hit, ent, &trace );
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t pm;
	gclient_t   *client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400; // faster than normal
		if ( client->ps.sprintExertTime ) {
			client->ps.speed *= 3;  // (SA) allow sprint in free-cam mode

		}
		// set up for pmove
		memset( &pm, 0, sizeof( pm ) );
		pm.ps = &client->ps;
		pm.pmext = &client->pmext;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		Pmove( &pm ); // JPW NERVE

		// Rafael - Activate
		// Ridah, made it a latched event (occurs on keydown only)
		if ( client->latched_buttons & BUTTON_ACTIVATE ) {
			Cmd_Activate_f( ent );
		}

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	if ( ent->flags & FL_NOFATIGUE ) {
		ent->client->ps.sprintTime = 20000;
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;

	// attack button cycles through spectators
	if ( ( client->buttons & BUTTON_ATTACK ) && !( client->oldbuttons & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	} else if (
		( client->sess.spectatorState == SPECTATOR_FOLLOW ) &&
		( ucmd->upmove > 0 ) &&
		( level.time - client->lastJump > 250 ) ) {
		client->lastJump = level.time;
		Cmd_FollowCycle_f( ent, -1 );
	} else if (
		( client->sess.sessionTeam == TEAM_SPECTATOR ) && // don't let dead team players do free fly
		( client->sess.spectatorState == SPECTATOR_FOLLOW ) &&
		( client->buttons & BUTTON_ACTIVATE ) &&
		!( client->oldbuttons & BUTTON_ACTIVATE ) ) {
		// code moved to StopFollowing
		StopFollowing( ent );
	}
}


/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( !g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove ||
				client->pers.cmd.rightmove ||
				client->pers.cmd.upmove ||
				( client->pers.cmd.wbuttons & WBUTTON_ATTACK2 ) ||
				( client->pers.cmd.buttons & BUTTON_ATTACK ) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t *client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// regenerate
		// JPW NERVE, split these completely
		if ( g_gametype.integer < GT_WOLF ) {
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health += 15;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
					ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// JPW NERVE
		else { // GT_WOLF
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health += 3;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 1.12 ) {
					ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.12 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.12;
					}
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// jpw
		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->pers.cmd.wbuttons;

	if ( ( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) ||
		 ( client->wbuttons & WBUTTON_ATTACK2 & ( client->oldwbuttons ^ client->wbuttons ) ) ) {
		client->readyToExit ^= 1;
	}
}

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int i;
	int event;
	gclient_t   *client;
	int damage;
	vec3_t dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & ( MAX_EVENTS - 1 ) ];

		switch ( event ) {
		case EV_FALL_NDIE:
		//case EV_FALL_SHORT:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_25:
		//case EV_FALL_DMG_30:
		case EV_FALL_DMG_50:
			//case EV_FALL_DMG_75:

			if ( ent->s.eType != ET_PLAYER ) {
				break;      // not in the player model
			}

			// TODO: clean this up ...
			if ( event == EV_FALL_NDIE ) {
				damage = 9999;
			} else if ( event == EV_FALL_DMG_50 ) {
				damage = 50;
				if ( !(g_dmflags.integer & DF_NO_FALLING) ) {
					ent->client->ps.pm_time = 1000;
					ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					VectorClear( ent->client->ps.velocity );
				}
			} else if ( event == EV_FALL_DMG_25 ) {
				damage = 25;
				if ( !(g_dmflags.integer & DF_NO_FALLING) ) {
					ent->client->ps.pm_time = 250;
					ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					VectorClear( ent->client->ps.velocity );
				}
			} else if ( event == EV_FALL_DMG_15 ) {
				damage = 15;
				if ( !(g_dmflags.integer & DF_NO_FALLING) ) {
					ent->client->ps.pm_time = 1000;
					ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					VectorClear( ent->client->ps.velocity );
				}
			} else if ( event == EV_FALL_DMG_10 ) {
				damage = 10;
				if ( !(g_dmflags.integer & DF_NO_FALLING) ) {
					ent->client->ps.pm_time = 1000;
					ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
					VectorClear( ent->client->ps.velocity );
				}
			} else {
				damage = 5; // never used

			}

			if ( ( g_goomba.integer ) && ( ent->s.groundEntityNum < MAX_CLIENTS ) && ( ent->s.groundEntityNum != -1 )
				 && ( g_entities[ent->s.groundEntityNum].client->ps.stats[STAT_HEALTH] > 0 ) ) {
				if ( g_goombaDmg.value ) {
					damage = g_goombaDmg.value;
				} else if ( damage > 24 ) {
					damage = 999;
				} else {
					damage += 50;
				}

				if ( !OnSameTeam( &g_entities[ent->s.groundEntityNum], ent ) ) {
					G_Damage( &g_entities[ent->s.groundEntityNum], ent, ent, tv( 0, 0, -1 ), NULL, damage, 0, MOD_GOOMBA ); //splat!

				}
				break;
			}

            // cs: moved down so goombas will still work.
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}

			VectorSet( dir, 0, 0, 1 );
			ent->pain_debounce_time = level.time + 200; // no normal pain sound
			G_Damage( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING );
			break;
// JPW NERVE
		case EV_TESTID1:
		case EV_TESTID2:
		case EV_ENDTEST:
			break;
// jpw
		case EV_FIRE_WEAPON_MG42:
			mg42_fire( ent );
			break;

		case EV_FIRE_WEAPON:
		case EV_FIRE_WEAPONB:
		case EV_FIRE_WEAPON_LASTSHOT:
			FireWeapon( ent );
			break;

//----(SA)	modified
		case EV_USE_ITEM1:      // ( HI_MEDKIT )	medkit
		case EV_USE_ITEM2:      // ( HI_WINE )		wine
		case EV_USE_ITEM3:      // ( HI_SKULL )		skull of invulnerable
		case EV_USE_ITEM4:      // ( HI_WATER )		protection from drowning
		case EV_USE_ITEM5:      // ( HI_ELECTRIC )	protection from electric attacks
		case EV_USE_ITEM6:      // ( HI_FIRE )		protection from fire attacks
		case EV_USE_ITEM7:      // ( HI_STAMINA )	restores fatigue bar and sets "nofatigue" for a time period
		case EV_USE_ITEM8:      // ( HI_BOOK1 )
		case EV_USE_ITEM9:      // ( HI_BOOK2 )
		case EV_USE_ITEM10:     // ( HI_BOOK3 )
			UseHoldableItem( ent, event - EV_USE_ITEM0 );
			break;
//----(SA)	end

		default:
			break;
		}
	}
}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
}

// DHM - Nerve
void WolfFindMedic( gentity_t *self ) {
	int i, medic = -1;
	gclient_t   *cl;
	vec3_t start, end, temp;
	trace_t tr;
	float bestdist = 1024, dist;

	self->client->ps.viewlocked_entNum = 0;
	self->client->ps.viewlocked = 0;
	self->client->ps.stats[STAT_DEAD_YAW] = 999;

	VectorCopy( self->s.pos.trBase, start );
	start[2] += self->client->ps.viewheight;

	for ( i = 0; i < level.numPlayingClients; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];

		if ( cl->ps.clientNum == self->client->ps.clientNum ) {
			continue;
		}
		if ( cl->sess.sessionTeam != self->client->sess.sessionTeam ) {
			continue;
		}
		if ( cl->ps.stats[ STAT_HEALTH ] <= 0 ) {
			continue;
		}
		if ( cl->ps.stats[ STAT_PLAYER_CLASS ] != PC_MEDIC ) {
			continue;
		}

		VectorCopy( g_entities[level.sortedClients[i]].s.pos.trBase, end );
		end[2] += cl->ps.viewheight;

		trap_Trace( &tr, start, NULL, NULL, end, self->s.number, CONTENTS_SOLID );
		if ( tr.fraction < 0.95 ) {
			continue;
		}

		VectorSubtract( end, start, end );
		dist = VectorNormalize( end );

		if ( dist < bestdist ) {
			medic = cl->ps.clientNum;
			vectoangles( end, temp );
			self->client->ps.stats[STAT_DEAD_YAW] = temp[YAW];
			bestdist = dist;
		}
	}

	if ( medic >= 0 ) {
		self->client->ps.viewlocked_entNum = medic;
		self->client->ps.viewlocked = 7;
	}
}

/*
============
GetLTinfo from sandmod
============
*/
void GetLTinfo( gentity_t *ent ) {
	int mainweapres = 0;
	int mainweapclip = 0;
	int pistweapres = 0;
	int pistweapclip = 0;
	int nadeammo = 0;
	int sidearm, i, weapon = 0;
	qboolean hasPrimary = qfalse;

	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

	if ( g_gamestate.integer != GS_PLAYING ) {
		return;
	}

	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	VectorCopy( ent->s.pos.trBase, start );   //set 'start' to the player's position (plus the viewheight)
	start[2] += ent->client->ps.viewheight;
	VectorMA( start, 512, forward, end );    //put 'end' 512 units forward of 'start'

	//see if we hit anything between 'start' and 'end'
	trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER ) );

	//if we didn't hit a player, return
	if ( tr.entityNum >= MAX_CLIENTS ) {
		return;
	}

	target = &g_entities[ tr.entityNum ];

	if ( ( !target->inuse ) || ( !target->client ) ) { //if the player is lagged/disconnected/etc
		return;
	}

	if ( target->client->ps.stats[STAT_HEALTH] <= 0 ) { //if they're dead
		return;
	}

	if ( !OnSameTeam( target,ent ) ) { //dont need to know otherteams info
		return;
	}

	ent->client->lastLTinfotime = level.time;

	// find the primary weapon
	for ( i = 0; i < MAX_WEAPS_IN_BANK_MP; i++ ) {
		weapon = weapBanksMultiPlayer[3][i];
		if ( COM_BitCheck( target->client->ps.weapons,weapon ) ) {
			hasPrimary = qtrue;
			mainweapres += target->client->ps.ammo[ BG_FindAmmoForWeapon( weapon ) ];
			mainweapclip += target->client->ps.ammoclip[ BG_FindClipForWeapon( weapon ) ];
			break;
		}
	}

	sidearm = target->client->sess.sessionTeam == TEAM_BLUE ? WP_COLT : WP_LUGER;
	pistweapres += target->client->ps.ammo[ BG_FindAmmoForWeapon( sidearm ) ];
	pistweapclip += target->client->ps.ammoclip[ BG_FindClipForWeapon( sidearm ) ];

	//add up total of grenades
	nadeammo += target->client->ps.ammoclip[ BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE ) ];
	nadeammo += target->client->ps.ammoclip[ BG_FindClipForWeapon( WP_GRENADE_LAUNCHER ) ];

	if ( hasPrimary ) {
		if ( weapon == WP_FLAMETHROWER ) {
			trap_SendServerCommand( ent - g_entities, va( "cp \"^3%s:^5%i ^3Pistol:^5%i/%i ^3Nades:^5%i\n\"",weaponToString( weapon ),mainweapclip,pistweapclip,pistweapres,nadeammo ) );
		} else {
			trap_SendServerCommand( ent - g_entities, va( "cp \"^3%s:^5%i/%i ^3Pistol:^5%i/%i ^3Nades:^5%i\n\"",weaponToString( weapon ),mainweapclip,mainweapres,pistweapclip,pistweapres,nadeammo ) );
		}
	} else {
		trap_SendServerCommand( ent - g_entities, va( "cp \"^3Pistol:^5%i/%i ^3Nades:^5%i\n\"",pistweapclip,pistweapres,nadeammo ) );
	}
}

void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE
void reinforce( gentity_t *ent ); // JPW NERVE

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
	gclient_t   *client;
	pmove_t pm;
	int oldEventSequence;
	int msec;
	usercmd_t   *ucmd;
	vec3_t muzzlebounce;
	gitem_t *item;
	gentity_t *ent2;
	vec3_t velocity;
	vec3_t angles,mins,maxs;
	trace_t tr;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if ( client->pers.connected != CON_CONNECTED ) {
		return;
	}

	if ( client->cameraPortal ) {
		G_SetOrigin( client->cameraPortal, client->ps.origin );
		trap_LinkEntity( client->cameraPortal );
		VectorCopy( client->cameraOrigin, client->cameraPortal->s.origin2 );
	}

	// lt info from sandmod
	if ( !( ent->r.svFlags & SVF_BOT ) && ( g_LTinfomsg.integer ) &&
		 ( ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) && ( level.time >= ent->client->lastLTinfotime + 1000 ) ) {
		GetLTinfo( ent );
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	ent->client->ps.identifyClient = ucmd->identClient;     // NERVE - SMF

	// zinx etpro antiwarp
	if ( client->warping && g_maxWarp.integer && G_DoAntiwarp( ent ) ) {
		int frames = ( level.framenum - client->lastUpdateFrame );

		if ( frames > g_maxWarp.integer ) {
			frames = g_maxWarp.integer;
		}
		// if the difference between commandTime and the last command
		// time is small, you won't move as far since it's doing
		// velocity*time for updating your position
		client->ps.commandTime = level.previousTime -
								 ( frames  * ( level.time - level.previousTime ) );
		client->warped = qtrue;
	}

	client->warping = qfalse;
	client->lastUpdateFrame = level.framenum;

// JPW NERVE -- update counter for capture & hold display
	if ( g_gametype.integer == GT_WOLF_CPH ) {
		client->ps.stats[STAT_CAPTUREHOLD_RED] = level.capturetimes[TEAM_RED];
		client->ps.stats[STAT_CAPTUREHOLD_BLUE] = level.capturetimes[TEAM_BLUE];
	}
// jpw

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 && !G_DoAntiwarp( ent ) ) {
		ucmd->serverTime = level.time + 200;
	}

	if ( ucmd->serverTime < level.time - 1000 && !G_DoAntiwarp( ent ) ) {
		ucmd->serverTime = level.time - 1000;
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
	}

	// Martin - fakeping
	// cs: lets do this once every couple seconds rather than multiple times a server frame ...
	if ( ent->r.svFlags & SVF_BOT && g_fakePing.integer && level.framenum % 40 == 0 ) {
		int flux = rand() % 10;
		flux -= 5;
		client->ps.ping = client->pers.seedPing + flux;
	}
	// end Martin

	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set( "pmove_msec", "8" );
	} else if ( pmove_msec.integer > 33 ) {
		trap_Cvar_Set( "pmove_msec", "33" );
	}

	// cs: added for zinx etpro antiwarp
	client->pers.pmoveMsec = pmove_msec.integer;

	// zinx etpro antiwarp
	if ( G_DoAntiwarp( ent ) && ( pmove_fixed.integer || client->pers.pmoveFixed ) ) {
		ucmd->serverTime = ( ( ucmd->serverTime + client->pers.pmoveMsec - 1 ) /
							 client->pers.pmoveMsec ) * client->pers.pmoveMsec;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	// DHM - Nerve :: In limbo use SpectatorThink
	if ( client->sess.sessionTeam == TEAM_SPECTATOR || client->ps.pm_flags & PMF_LIMBO ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD /*|| (ent->r.svFlags & SVF_BOT)*/ ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// JPW NERVE do some time-based muzzle flip -- this never gets touched in single player (see g_weapon.c)
	// #define RIFLE_SHAKE_TIME 150 // JPW NERVE this one goes with the commented out old damped "realistic" behavior below
	#define RIFLE_SHAKE_TIME 300 // per Id request, longer recoil time
	if ( client->sniperRifleFiredTime ) {
		if ( level.time - client->sniperRifleFiredTime > RIFLE_SHAKE_TIME ) {
			client->sniperRifleFiredTime = 0;
		} else {
			VectorCopy( client->ps.viewangles,muzzlebounce );

			// JPW per Id request, longer recoil time
			muzzlebounce[PITCH] -= 2 * cos( 2.5 * ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			muzzlebounce[YAW] += 0.5*client->sniperRifleMuzzleYaw*cos( 1.0 - ( level.time - client->sniperRifleFiredTime )*3 / RIFLE_SHAKE_TIME );
			muzzlebounce[PITCH] -= 0.25 * random() * ( 1.0f - ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			muzzlebounce[YAW] += 0.5 * crandom() * ( 1.0f - ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			SetClientViewAngle( ent,muzzlebounce );
		}
	}
	if ( client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ) {
		if ( level.time > client->ps.powerups[PW_REGEN] + 5000 ) {
			client->ps.powerups[PW_REGEN] = level.time;
		}
	}
	// also update weapon recharge time

	// JPW drop button drops secondary weapon so new one can be picked up
	if ( ucmd->wbuttons & WBUTTON_DROP ) {
		if ( !client->dropWeaponTime ) {
			client->dropWeaponTime = 1; // just latch it for now

			if ( client->ps.weapon == WP_KNIFE ) {
				if ( ( g_throwableKnives.integer > 0 ) || ( g_throwableKnives.integer == -1 ) ) {
					Cmd_ThrowKnife_f( ent );
				}
			}
			else {
				int i, weapon;
				vec3_t offset, org;

				for ( i = 0; i < MAX_WEAPS_IN_BANK_MP; i++ )
				{
					weapon = weapBanksMultiPlayer[3][i];
					if ( COM_BitCheck( client->ps.weapons,weapon ) ) {

						item = BG_FindItemForWeapon( (weapon_t)weapon );
						if ( item ) {
							VectorCopy( client->ps.viewangles, angles );

							// clamp pitch
							if ( angles[PITCH] < -30 ) {
								angles[PITCH] = -30;
							} else if ( angles[PITCH] > 30 ) {
								angles[PITCH] = 30;
							}

							AngleVectors( angles, velocity, NULL, NULL );
							VectorScale( velocity, 64, offset );
							offset[2] += client->ps.viewheight / 2;
							VectorScale( velocity, 75, velocity );
							velocity[2] += 50 + random() * 35;

							VectorAdd( client->ps.origin,offset,org );

							VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
							VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS );

							trap_Trace( &tr, client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID );
							VectorCopy( tr.endpos, org );

							ent2 = LaunchItem( item, org, velocity, ent-g_entities );
							COM_BitClear( client->ps.weapons,weapon );

							if ( weapon == WP_MAUSER ) {
								COM_BitClear( client->ps.weapons,WP_SNIPERRIFLE );
							}

							// Clear out empty weapon, change to next best weapon
							G_AddEvent( ent, EV_NOAMMO, 0 );

							i = MAX_WEAPS_IN_BANK_MP;
							// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=568
							if ( client->ps.weapon == weapon ) {
								client->ps.weapon = 0;
							}
							ent2->count = client->ps.ammoclip[BG_FindClipForWeapon( (weapon_t)weapon )];
							ent2->item->quantity = client->ps.ammo[BG_FindClipForWeapon( (weapon_t)weapon )];
							client->ps.ammoclip[BG_FindClipForWeapon( (weapon_t)weapon )] = 0;
							client->ps.ammo[BG_FindClipForWeapon( (weapon_t)weapon )] = 0;

							Bot_Event_RemoveWeapon( client->ps.clientNum, Bot_WeaponGameToBot( weapon ) );
						}
					}
				}
			}
		}
	} else {
		client->dropWeaponTime = 0;
	}
// jpw

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	if ( reloading || client->cameraPortal ) {
		ucmd->buttons = 0;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		ucmd->wbuttons = 0;
		ucmd->wolfkick = 0;
		if ( client->cameraPortal ) {
			client->ps.pm_type = PM_FREEZE;
		}
	} else if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	// set parachute anim condition flag
	BG_UpdateConditionValue( ent->s.number, ANIM_COND_PARACHUTE, ( ent->flags & FL_PARACHUTE ) != 0, qfalse );

	// all playing clients are assumed to be in combat mode
	if ( !client->ps.aiChar ) {
		client->ps.aiState = AISTATE_COMBAT;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	client->currentAimSpreadScale = (float)client->ps.aimSpreadScale / 255.0;

	memset( &pm, 0, sizeof( pm ) );

	pm.ps = &client->ps;
	pm.pmext = &client->pmext;
	pm.cmd = *ucmd;
	pm.oldcmd = client->pers.oldcmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		pm.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	} else {
		pm.tracemask = MASK_PLAYERSOLID;
	}

	//DHM - Nerve :: We've gone back to using normal bbox traces
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	// fixed physics from sandmod
	if ( ent->r.svFlags & SVF_BOT ) {
		pm.fixedphysics = 1;
		pm.fixedphysicsfps = 125;
	} else
	{
		pm.fixedphysics = g_fixedphysics.integer;
		if ( g_fixedphysics.integer == 2 ) {
			pm.fixedphysicsfps = 333;
		} else if ( g_fixedphysics.integer == 1 ) {
			pm.fixedphysicsfps = 125;
		}
	}

	pm.noWeapClips = ( g_dmflags.integer & DF_NO_WEAPRELOAD ) > 0;

	VectorCopy( client->ps.origin, client->oldOrigin );

	// NERVE - SMF
	pm.gametype = g_gametype.integer;
	pm.ltChargeTime = g_LTChargeTime.integer;
	pm.soldierChargeTime = g_soldierChargeTime.integer;
	pm.engineerChargeTime = g_engineerChargeTime.integer;
	pm.medicChargeTime = g_medicChargeTime.integer;
	// -NERVE - SMF

	Pmove( &pm );

	// server cursor hints
	if ( !(ent->r.svFlags & SVF_BOT) && ent->lastHintCheckTime < level.time ) {
		G_CheckForCursorHints( ent );
		ent->lastHintCheckTime = level.time + FRAMETIME;
	}

	// DHM - Nerve :: Set animMovetype to 1 if ducking
	if ( ent->client->ps.pm_flags & PMF_DUCKED ) {
		ent->s.animMovetype = 1;
	} else {
		ent->s.animMovetype = 0;
	}

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy( pm.mins, ent->r.mins );
	VectorCopy( pm.maxs, ent->r.maxs );

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity( ent );
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons = client->buttons & ~client->oldbuttons;

	//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;
	client->latched_wbuttons = client->wbuttons & ~client->oldwbuttons;

	// Rafael - Activate
	// Ridah, made it a latched event (occurs on keydown only)
	if ( client->latched_buttons & BUTTON_ACTIVATE ) {
		Cmd_Activate_f( ent );
	}

	if ( ent->flags & FL_NOFATIGUE ) {
		ent->client->ps.sprintTime = 20000;
	}

	// Omni-bot: used for class changes, bot will /kill 2 seconds before spawn
	Bot_Util_CheckForSuicide( ent );

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {

		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF ) {
			WolfFindMedic( ent );
		}
		// dhm - end

		// See if we need to hop to limbo
		if( level.time > client->respawnTime && !(ent->client->ps.pm_flags & PMF_LIMBO) ) {
			if( ucmd->upmove > 0 ) {
				limbo( ent, ( client->ps.stats[STAT_HEALTH] > GIB_HEALTH ) );
				if ( g_reportTaps.integer ) {
					trap_SendServerCommand( -1, va( "print \"%s ^7tapped out\n\"", ent->client->pers.netname ) );
				}
			}
			else if((g_forcerespawn.integer > 0 && level.time - client->respawnTime > g_forcerespawn.integer * 1000) || client->ps.stats[STAT_HEALTH] <= GIB_HEALTH) {
				limbo(ent, (client->ps.stats[STAT_HEALTH] > GIB_HEALTH));
			}
		}

		return;
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );

	if ( g_debugPlayerHitboxes.integer & 2 && ent-g_entities != 0 ) {
		gentity_t *head;

		VectorCopy( ent->r.maxs, maxs );
		G_RailBox( ent->r.currentOrigin, ent->r.mins, maxs, tv( 0.f,0.f,1.f ), ent-g_entities );

		head = G_BuildHead( ent );
		G_RailBox( head->r.currentOrigin, head->r.mins, head->r.maxs, tv( 0.f,0.f,1.f ), head->s.number | HITBOXBIT_HEAD );
		G_FreeEntity( head );
	}
}

// zinx etpro antiwarp
void ClientThink_cmd( gentity_t *ent, usercmd_t *cmd ) {
	ent->client->pers.oldcmd = ent->client->pers.cmd;
	ent->client->pers.cmd = *cmd;
	ClientThink_real( ent );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	ent->client->pers.oldcmd = ent->client->pers.cmd;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

#ifdef ALLOW_GSYNC
	if ( !g_synchronousClients.integer )
#endif
	{
		if ( G_DoAntiwarp( ent ) ) {
			// zinx etpro antiwarp
			etpro_AddUsercmd( clientNum, &ent->client->pers.cmd );
			DoClientThinks( ent );
		} else {
			ClientThink_cmd( ent, &ent->client->pers.cmd );
		}
	}
}


void G_RunClient( gentity_t *ent ) {
	if ( ent->client->pers.cmd.buttons & BUTTON_ACTIVATE ) {
		Cmd_Activate2_f( ent );
	}
#ifdef ALLOW_GSYNC
	if ( !g_synchronousClients.integer )
#endif
	{
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;

	if ( G_DoAntiwarp( ent ) ) {
		DoClientThinks( ent );
	} else {
		ClientThink_real( ent );
	}
}

/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) || ( ent->client->ps.pm_flags & PMF_LIMBO ) ) { // JPW NERVE for limbo
		int clientNum, testtime, do_respawn = 0;

		// Players can respawn quickly in warmup
		if ( g_gamestate.integer != GS_PLAYING && /*ent->client->respawnTime <= level.timeCurrent &&*/
			 ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			do_respawn = 1;
		} else if ( ent->client->sess.sessionTeam == TEAM_RED ) {
			testtime = ( level.dwRedReinfOffset + level.time - level.startTime ) % g_redlimbotime.integer;
			if ( testtime < ent->client->pers.lastReinforceTime ) {
				do_respawn = 1;
			}
			ent->client->pers.lastReinforceTime = testtime;
		} else if ( ent->client->sess.sessionTeam == TEAM_BLUE ) {
			testtime = ( level.dwBlueReinfOffset + level.time - level.startTime ) % g_bluelimbotime.integer;
			if ( testtime < ent->client->pers.lastReinforceTime ) {
				do_respawn = 1;
			}
			ent->client->pers.lastReinforceTime = testtime;
		}

		if ( ( g_maxlives.integer > 0 || g_alliedmaxlives.integer > 0 || g_axismaxlives.integer > 0 ) && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 ) {
			do_respawn = 0;
		}

		if ( do_respawn ) {
			reinforce( ent );
			return;
		}

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			gclient_t   *cl;
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				// DHM - Nerve :: carry flags over
				int flags = ( cl->ps.eFlags & ~( EF_VOTED ) ) | ( ent->client->ps.eFlags & ( EF_VOTED ) );
				// JPW NERVE -- limbo latch
				if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->ps.pm_flags & PMF_LIMBO ) {
					// abuse do_respawn var
					int savedScore = ent->client->ps.persistant[PERS_SCORE];
					int savedRespawns = ent->client->ps.persistant[PERS_RESPAWNS_LEFT];
					int savedClass = ent->client->ps.stats[STAT_PLAYER_CLASS];
					do_respawn = ent->client->ps.pm_time;

					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
					ent->client->ps.pm_flags |= PMF_LIMBO;

					ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = savedRespawns;
					ent->client->ps.pm_time = do_respawn;                           // put pm_time back
					ent->client->ps.persistant[PERS_SCORE] = savedScore;            // put score back
					ent->client->ps.stats[STAT_PLAYER_CLASS] = savedClass;          // NERVE - SMF - put player class back
				} else {
					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
				}
				// jpw
				// DHM - Nerve :: carry flags over
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}


// DHM - Nerve :: After reviving a player, their contents stay CONTENTS_CORPSE until it is determined
//					to be safe to return them to PLAYERSOLID

qboolean StuckInClient( gentity_t *self ) {
	int i;
	vec3_t hitmin, hitmax;
	vec3_t selfmin, selfmax;
	gentity_t *hit;

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		hit = g_entities + level.sortedClients[i];

		if ( !hit->inuse || hit == self || !hit->client ||
			 !hit->s.solid || hit->health <= 0 ) {
			continue;
		}

		VectorAdd( hit->r.currentOrigin, hit->r.mins, hitmin );
		VectorAdd( hit->r.currentOrigin, hit->r.maxs, hitmax );
		VectorAdd( self->r.currentOrigin, self->r.mins, selfmin );
		VectorAdd( self->r.currentOrigin, self->r.maxs, selfmax );

		if ( hitmin[0] > selfmax[0] ) {
			continue;
		}
		if ( hitmax[0] < selfmin[0] ) {
			continue;
		}
		if ( hitmin[1] > selfmax[1] ) {
			continue;
		}
		if ( hitmax[1] < selfmin[1] ) {
			continue;
		}
		if ( hitmin[2] > selfmax[2] ) {
			continue;
		}
		if ( hitmax[2] < selfmin[2] ) {
			continue;
		}

		return( qtrue );
	}

	return( qfalse );
}

extern vec3_t playerMins, playerMaxs;
#define WR_PUSHAMOUNT 25

void WolfRevivePushEnt( gentity_t *self, gentity_t *other ) {
	vec3_t dir, push;

	VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
	dir[2] = 0;
	VectorNormalizeFast( dir );

	VectorScale( dir, WR_PUSHAMOUNT, push );

	if ( self->client ) {
		VectorAdd( self->s.pos.trDelta, push, self->s.pos.trDelta );
		VectorAdd( self->client->ps.velocity, push, self->client->ps.velocity );
	}

	VectorScale( dir, -WR_PUSHAMOUNT, push );
	push[2] = WR_PUSHAMOUNT / 2;

	VectorAdd( other->s.pos.trDelta, push, other->s.pos.trDelta );
	if ( other->client ) {
		VectorAdd( other->client->ps.velocity, push, other->client->ps.velocity );
	}
}

void WolfReviveBbox( gentity_t *self ) {
	int touch[MAX_GENTITIES];
	int num,i, touchnum = 0;
	gentity_t   *hit = NULL;
	vec3_t mins, maxs;

	hit = G_TestEntityPosition( self );

	if ( hit && ( hit->s.number == ENTITYNUM_WORLD || ( hit->client && hit->client->ps.persistant[PERS_HWEAPON_USE] ) ) ) {
		G_DPrintf( "WolfReviveBbox: Player stuck in world or MG42 using player\n" );
		// Move corpse directly to the person who revived them
		if ( self->props_frame_state >= 0 ) {
			VectorCopy( g_entities[self->props_frame_state].client->ps.origin, self->client->ps.origin );
			VectorCopy( self->client->ps.origin, self->r.currentOrigin );
			trap_LinkEntity( self );

			// Reset value so we don't continue to warp them
			self->props_frame_state = -1;
		}
		return;
	}

	VectorAdd( self->r.currentOrigin, playerMins, mins );
	VectorAdd( self->r.currentOrigin, playerMaxs, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];

		// Always use capsule for player
		if ( !trap_EntityContactCapsule( mins, maxs, hit ) ) {
			continue;
		}

		if ( hit->client && hit->health > 0 ) {
			if ( hit->s.number != self->s.number ) {
				WolfRevivePushEnt( hit, self );
				touchnum++;
			}
		} else if ( hit->r.contents & ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_PLAYERCLIP ) ) {
			WolfRevivePushEnt( hit, self );
			touchnum++;
		}
	}

	G_DPrintf( "WolfReviveBbox: Touchnum: %d\n", touchnum );

	if ( touchnum == 0 ) {
		G_DPrintf( "WolfReviveBbox:  Player is solid now!\n" );
		self->r.contents = CONTENTS_BODY;
	}
}

// dhm

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEndFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int frames;
	int i;

	if ( ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) || ( ent->client->ps.pm_flags & PMF_LIMBO ) ) { // JPW NERVE
		SpectatorClientEndFrame( ent );
		return;
	}

    // turn off any expired powerups
    for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {

        if ( i == PW_FIRE ||             // these aren't dependant on level.time
             i == PW_ELECTRIC ||
             i == PW_BREATHER ||
             i == PW_NOFATIGUE ) {

            continue;
        }

        if ( ent->client->ps.powerups[ i ] < level.time ) {
            ent->client->ps.powerups[ i ] = 0;
        }
    }

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects( ent );

	// apply all the damage taken this frame
	P_DamageFeedback( ent );

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;   // FIXME: get rid of ent->health...

	G_SetClientSound( ent );

	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, level.time, qfalse );

	// DHM - Nerve :: If it's been a couple frames since being revived, and props_frame_state
	//					wasn't reset, go ahead and reset it
	if ( ent->props_frame_state >= 0 && ( ( level.time - ent->s.effect3Time ) > 100 ) ) {
		ent->props_frame_state = -1;
	}

	if ( ent->health > 0 && StuckInClient( ent ) ) {
		G_DPrintf( "%s is stuck in a client.\n", ent->client->pers.netname );
		ent->r.contents = CONTENTS_CORPSE;
	}

	if ( g_gametype.integer >= GT_WOLF && ent->health > 0 && ent->r.contents == CONTENTS_CORPSE ) {
		WolfReviveBbox( ent );
	}

	// DHM - Nerve :: Reset 'count2' for flamethrower
	if ( !( ent->client->buttons & BUTTON_ATTACK ) ) {
		ent->count2 = 0;
	}
	// dhm

	// zinx etpro antiwarp
	frames = level.framenum - ent->client->lastUpdateFrame - 1;
	if ( g_maxWarp.integer && frames > g_maxWarp.integer && G_DoAntiwarp( ent ) ) {
		ent->client->warping = qtrue;
	}

	ent->client->warped = qfalse;

	if ( g_debugPlayerHitboxes.integer & 1 && ent-g_entities != 0 ) {
		gentity_t *head;
		vec3_t maxs;

		VectorCopy( ent->r.maxs, maxs );
		G_RailBox( ent->r.currentOrigin, ent->r.mins, maxs, tv( 0.f,1.f,0.f ), ent-g_entities );

		head = G_BuildHead( ent );
		G_RailBox( head->r.currentOrigin, head->r.mins, head->r.maxs, tv( 0.f,1.f,0.f ), ent->s.number | HITBOXBIT_HEAD );
		G_FreeEntity( head );
	}

	// store the client's current position for antilag traces
	G_StoreClientPosition( ent );
}
