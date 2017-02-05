
#include "gamestate.h"

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

int InitializeGameState( GameState* state )
{
    state->pool = AllocateMemoryPool( 2 * MB );
    if ( !state->pool )
    {
        return 0;
    }

    state->entity_section = AddSection( state->pool, 500 * KB );
    state->transient_section = AddSection( state->pool, 0 );

    state->gridleft = -100;
    state->gridtop = -100;
    state->gridright = 100;
    state->gridbottom = 100;

    state->game_mode = 0;

    state->grid_m = 1.0;
    state->screenw_m = 25;
    state->screenh_m = 20;

    state->logical_frames = 0;
    state->fps = 0;

    state->camera_follow = NULL;

    state->free_entity = NULL;

    state->entities = state->pool->sections[state->entity_section].begin;
    state->camera_follow = AddBlank( state, 0, 0 );
    AddPlayer( state, 0, 0, 0 );

    state->numentities = 2;
    return 1;
}

static Entity* GetFreeEntity( GameState* state )
{
    Entity* result;
    if ( state->free_entity )
    {
        result = state->free_entity;
        state->free_entity = state->free_entity->next;
    }
    else
    {
        result = PUSH_STRUCT_INDEX( Entity, state->pool, state->entity_section );
    }

    result->grid_x = 0.0f;
    result->grid_y = 0.0f;
    result->off_x = 0.0f;
    result->off_y = 0.0f;
    result->dx = 0.0f;
    result->dy = 0.0f;
    result->ddx = 0.0f;
    result->ddy = 0.0f;

    result->flags = ENTITY_ACTIVE;

    result->orient = OrientDown;

    return result;
}

Entity* AddBlank( GameState* state, int grid_x, int grid_y )
{
    Entity* blank = GetFreeEntity( state );

    blank->grid_x = grid_x;
    blank->grid_y = grid_y;

    blank->flags |= ENTITY_INVISIBLE;

    return blank;
}

Entity* AddPlayer( GameState* state, int grid_x, int grid_y, int input_index )
{
    Entity* player_entity = GetFreeEntity( state );

    player_entity->grid_x = grid_x;
    player_entity->grid_y = grid_y;

    player_entity->flags |= ENTITY_PLAYER;

    return player_entity;
}

Entity* AddWall( GameState* state, int grid_x, int grid_y )
{
    Entity* wall = GetFreeEntity( state );
    wall->grid_x = grid_x;
    wall->grid_y = grid_y;

    wall->flags |= ENTITY_COLLIDES;

    return wall;
}

void DestroyEntity( GameState* state, Entity* entity )
{
    entity->flags &= ~ENTITY_ACTIVE;
    entity->next = state->free_entity;
    state->free_entity = entity;
}

void UpdateGameState( GameState* state, float elapsed )
{
    int i;
    for ( i = 0; i < state->numentities; i++ )
    {
        Entity* entity = &state->entities[i];

        entity->off_x += entity->dx;
        entity->off_y += entity->dy;
        if ( entity->off_x >= state->grid_m / 2 )
        {
            entity->dx = 0;
            entity->grid_x++;
            entity->off_x -= state->grid_m;
        }
        else if ( entity->off_x <= -state->grid_m / 2 )
        {
            entity->dx = 0;
            entity->grid_x--;
            entity->off_x += state->grid_m;
        }

        if ( entity->off_y >= state->grid_m / 2 )
        {
            entity->dy = 0;
            entity->grid_y++;
            entity->off_y -= state->grid_m;
        }
        else if ( entity->off_y <= -state->grid_m / 2 )
        {
            entity->dy = 0;
            entity->grid_y--;
            entity->off_y += state->grid_m;
        }

        if ( entity->dy == 0.0f && entity->dx == 0.0f )
        {
#define MOVEMENT_SPEED 1.0f

            if ( entity->flags & ENTITY_PLAYERCONTROLLED )
            {
                if ( state->in[entity->controller].keydown[KeyUp] )
                {
                    entity->dy = -MOVEMENT_SPEED;
                    state->in[entity->controller].keydown[KeyUp] = 0;
                }
                else if ( state->in[entity->controller].keydown[KeyDown] )
                {
                    entity->dy = MOVEMENT_SPEED;
                    state->in[entity->controller].keydown[KeyDown] = 0;
                }

                if( state->in[entity->controller].keydown[KeyLeft] )
                {
                    entity->dx = -MOVEMENT_SPEED;
                    state->in[entity->controller].keydown[KeyLeft] = 0;
                }
                else if (state->in[entity->controller].keydown[KeyRight])
                {
                    entity->dx = MOVEMENT_SPEED;
                    state->in[entity->controller].keydown[KeyRight] = 0;
                }
            }
        }
    }
}

void RenderGameState( Bitmap* screen, const Rect* dstrect, GameState* state, float elapsed )
{
    Rect dst_local;
    if ( dstrect )
    {
        dst_local = *dstrect;
    }
    else
    {
        dst_local.x = 0;
        dst_local.y = 0;
        dst_local.w = screen->w;
        dst_local.h = screen->h;
    }

    float camera_off_x = 0;
    float camera_off_y = 0;
    float camera_x = 0;
    float camera_y = 0;
    if ( state->camera_follow && state->camera_follow->flags & ENTITY_ACTIVE )
    {
        camera_off_x = state->camera_follow->off_x;
        camera_off_y = state->camera_follow->off_y;
        camera_x = state->camera_follow->grid_x;
        camera_y = state->camera_follow->grid_y;
    }

    float ppmw = dst_local.w / state->screenw_m;
    float ppmh = dst_local.h / state->screenh_m;
    float left_edge = -state->screenw_m / 2 + camera_off_x;
    float top_edge = -state->screenh_m / 2 + camera_off_y;

    RGB white;
    white.r = 255;
    white.g = 255;
    white.b = 255;

    float x, y;
    for ( x = -(int)(state->screenw_m / 2); x < (int)(state->screenw_m / 2) + 1; x += state->grid_m )
    {
        DrawVerticalLine( screen, 0, dst_local.h - 1, (x - left_edge) * ppmw, white );
    }

    for ( y = -(int)(state->screenh_m / 2); y < (int)(state->screenh_m / 2) + 1; y += state->grid_m )
    {
        DrawHorizontalLine( screen, 0, dst_local.w - 1, (y - top_edge) * ppmh, white );
    }

    int i;
    for ( i = 0; i < state->numentities; i++ )
    {
        Entity* entity = state->entities + i;
        if ( entity && entity->flags & ENTITY_ACTIVE && !(entity->flags & ENTITY_INVISIBLE))
        {
            int relgrid_x = entity->grid_x - camera_x;
            int relgrid_y = entity->grid_y - camera_y;

            Rect dstentity;
            dstentity.x = (relgrid_x + entity->off_x + entity->dx * elapsed - left_edge) * ppmw; //+ ppmw * state->screenw_m / 2;// + entity->off_x;
            dstentity.y = (relgrid_y + entity->off_y + entity->dy * elapsed - top_edge) * ppmh;// + ppmh * state->screenh_m / 2;
            dstentity.w = state->grid_m * ppmw;
            dstentity.h = state->grid_m * ppmh;

            RGB color;
            color.r = 0;
            color.g = 255;
            color.b = 127;

            FillRectangle( screen, &dstentity, color );
        }
    }

}
