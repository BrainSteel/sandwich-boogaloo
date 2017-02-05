
#include "windows.h"

#include "common.h"
#include "xorshiftstar.h"
#include "gamestate.h"
#include "pixel.h"

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

    state->grid_m = 1.0;
    state->screenw_m = 25;
    state->screenh_m = 20;

    state->logical_frames = 0;
    state->fps = 0;
    state->rendering_enabled = 1;

    state->player_score = 0;

    RGB colorkey;
    colorkey.r = 255;
    colorkey.g = 255;
    colorkey.b = 255;

    if ( !LoadImageFromFile( "Spr_BreadSlice.bmp", &state->textures.bread, &colorkey ))
    {
        return 0;
    }

    if ( !CreateImage( &state->textures.player_texture, state->textures.bread.w, 1000 ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "Spr_SandWitch.bmp", &state->textures.witch, &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "Spr_Crecent.bmp", &state->textures.items[ItemCrecent], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "Spr_Lettuce.bmp", &state->textures.items[ItemLettuce], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "Spr_Quadropus.bmp", &state->textures.items[ItemQuadropus], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "Spr_Tomato.bmp", &state->textures.items[ItemTomato], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "T_BoneSand.bmp", &state->textures.tiles[TileBoneSand], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "T_SesameSand.bmp", &state->textures.tiles[TileSesameSand], &colorkey ))
    {
        return 0;
    }

    if ( !LoadImageFromFile( "T_WavySand.bmp", &state->textures.tiles[TileWavySand], &colorkey ))
    {
        return 0;
    }

    state->camera_follow = NULL;
    state->free_entity = NULL;

    state->entities = state->pool->sections[state->entity_section].begin;
    state->camera_follow = AddBlank( state, 0, 0 );

    GenerateLevel( state );

    AddPlayer( state, -7, 0, 0 );

    AddWitch( state, 0, 0 );

    return 1;
}

void GenerateLevel( GameState* state )
{
    state->textures.tiletype = xorshift64star_uniform( TileNum );

    int prevtop = 0;
    int prevbot = 0;

    int top = 0;
    int bot = 0;

    AddWall( state, -8, 0 );

    int midbot = 0;

    int x, y;

    for ( y = -6; y < 6; y++ )
    {
        AddWall( state, -8, y );
    }

    for( x = -7; x <= 7; x++ )
    {
        top = xorshift64star_range( 1, 5 );
        bot = -xorshift64star_range( 1, 5 );

        for ( y = -6; y < bot; y++ )
        {
            AddWall( state, x, y );
        }

        for ( y = top; y < 6; y++ )
        {
            AddWall( state, x, y );
        }

        //AddWall( state, x, bot );
        //AddWall( state, x, top );

        for ( y = bot; y < top; y++ )
        {
            AddTile( state, x, y );
            if ( xorshift64star_uniform( 10 ) == 0 )
            {
                AddItem( state, x, y );
            }
        }

        if ( x == 0 )
        {
            midbot = bot;
        }

        prevbot = bot;
        prevtop = top;
    }

    AddCrecent( state, 7, prevtop - 1 );

    for ( y = -6; y < 6; y++ )
    {
        AddWall( state, 8, y );
    }
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
        state->numentities++;
    }

    result->grid_x = 0.0f;
    result->grid_y = 0.0f;
    result->off_x = 0.0f;
    result->off_y = 0.0f;
    result->dx = 0.0f;
    result->dy = 0.0f;

    result->score = 0;
    result->paralyzed_duration = 0;
    result->type = 0;

    result->flags = ENTITY_ACTIVE;

    return result;
}

Entity* AddBlank( GameState* state, int grid_x, int grid_y )
{
    Entity* blank = GetFreeEntity( state );

    blank->grid_x = grid_x;
    blank->grid_y = grid_y;

    blank->flags |= ENTITY_INVISIBLE;
    blank->texture = NULL;

    return blank;
}

Entity* AddPlayer( GameState* state, int grid_x, int grid_y, int input_index )
{
    Entity* player_entity = GetFreeEntity( state );

    player_entity->grid_x = grid_x;
    player_entity->grid_y = grid_y;

    player_entity->flags |= ENTITY_PLAYER;

    player_entity->texture = &state->textures.player_texture;
    player_entity->texture_rect.x = 0;
    player_entity->texture_rect.y = player_entity->texture->h - player_entity->texture->w;
    player_entity->texture_rect.w = player_entity->texture->w;
    player_entity->texture_rect.h = player_entity->texture->w;

    ClearBitmap( player_entity->texture, WHITE );
    ImageBlitScaled( &state->textures.bread, player_entity->texture, NULL, &player_entity->texture_rect, AlphaBinary );

    return player_entity;
}

Entity* AddWall( GameState* state, int grid_x, int grid_y )
{
    Entity* wall = GetFreeEntity( state );
    wall->grid_x = grid_x;
    wall->grid_y = grid_y;

    wall->flags |= ENTITY_COLLIDES | ENTITY_INVISIBLE;

    return wall;
}

Entity* AddTile( GameState* state, int grid_x, int grid_y )
{
    Entity* tile = GetFreeEntity( state );
    tile->grid_x = grid_x;
    tile->grid_y = grid_y;

    tile->texture = &state->textures.tiles[state->textures.tiletype];
    tile->texture_rect.x = 0;
    tile->texture_rect.y = 0;
    tile->texture_rect.w = tile->texture->w;
    tile->texture_rect.h = tile->texture->h;

    return tile;
}

Entity* AddItem( GameState* state, int grid_x, int grid_y )
{
    Entity* item = GetFreeEntity( state );
    item->grid_x = grid_x;
    item->grid_y = grid_y;

    item->flags |= ENTITY_INTERACTIVE | ENTITY_ITEM;

    int rnd = xorshift64star_uniform( 10 );
    if ( rnd == 0 )
    {
        item->type = ItemQuadropus;
        item->score = 100;
    }
    else if ( rnd < 5 )
    {
        item->type = ItemTomato;
        item->score = 30;
    }
    else
    {
        item->type = ItemLettuce;
        item->score = 20;
    }

    item->texture = &state->textures.items[item->type];
    item->texture_rect.x = 0;
    item->texture_rect.y = 0;
    item->texture_rect.w = item->texture->w;
    item->texture_rect.h = item->texture->h;

    return item;
}

Entity* AddCrecent( GameState* state, int grid_x, int grid_y )
{
    Entity* crecent = GetFreeEntity( state );
    crecent->grid_x = grid_x;
    crecent->grid_y = grid_y;

    crecent->flags |= ENTITY_INTERACTIVE | ENTITY_ITEM;

    crecent->type = ItemCrecent;
    crecent->score = 75;

    crecent->texture = &state->textures.items[ItemCrecent];
    crecent->texture_rect.x = 0;
    crecent->texture_rect.y = 0;
    crecent->texture_rect.w = crecent->texture->w;
    crecent->texture_rect.h = crecent->texture->h;

    return crecent;
}

Entity* AddWitch( GameState* state, int grid_x, int grid_y )
{
    Entity* witch = GetFreeEntity( state );
    witch->grid_x = grid_x;
    witch->grid_y = grid_y;

    witch->flags |= ENTITY_SAND_WITCH;

    witch->texture = &state->textures.witch;
    witch->texture_rect.x = 0;
    witch->texture_rect.y = 0;
    witch->texture_rect.w = witch->texture->w;
    witch->texture_rect.h = witch->texture->h;

    return witch;
}

void DestroyEntity( GameState* state, Entity* entity )
{
    entity->flags &= ~ENTITY_ACTIVE;
    entity->next = state->free_entity;
    state->free_entity = entity;
}

void UpdateGameState( GameState* state, float elapsed )
{
    int i, j;
    for ( i = 0; i < state->numentities; i++ )
    {
        Entity* entity = &state->entities[i];

        if ( entity->flags & ENTITY_MOVES )
        {
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
        }
    }

    for ( i = 0; i < state->numentities; i++ )
    {
        Entity* entity = &state->entities[i];

        if ( entity->flags & ENTITY_MOVES )
        {
            int blocks[4] = {0, 0, 0, 0};
            if ( entity->flags & ENTITY_COLLIDES )
            {
                for ( j = 0; j < state->numentities; j++ )
                {
                    Entity* secondary = &state->entities[j];
                    if ( secondary->flags & ENTITY_ACTIVE )
                    {
                        if ( secondary->flags & ENTITY_COLLIDES && !(secondary->flags & ENTITY_PLAYERCONTROLLED))
                        {
                            if ( secondary->grid_x == entity->grid_x &&
                                 secondary->grid_y == entity->grid_y - 1 )
                            {
                                blocks[KeyUp] = 1;

                                if ( entity->flags & ENTITY_PLAYERCONTROLLED )
                                {
                                    state->in[0].keydown[KeyUp] = 0;
                                }
                            }
                            else if ( secondary->grid_x == entity->grid_x &&
                                      secondary->grid_y == entity->grid_y + 1 )
                            {
                                blocks[KeyDown] = 1;
                                if ( entity->flags & ENTITY_PLAYERCONTROLLED )
                                {
                                    state->in[0].keydown[KeyDown] = 0;
                                }
                            }
                            else if ( secondary->grid_x == entity->grid_x - 1 &&
                                      secondary->grid_y == entity->grid_y )
                            {
                                blocks[KeyLeft] = 1;
                                if ( entity->flags & ENTITY_PLAYERCONTROLLED )
                                {
                                    state->in[0].keydown[KeyLeft] = 0;
                                }
                            }
                            else if ( secondary->grid_x == entity->grid_x + 1 &&
                                      secondary->grid_y == entity->grid_y )
                            {
                                blocks[KeyRight] = 1;
                                if ( entity->flags & ENTITY_PLAYERCONTROLLED )
                                {
                                    state->in[0].keydown[KeyRight] = 0;
                                }
                            }
                        }

                        if ( entity->flags & ENTITY_PLAYERCONTROLLED &&
                             secondary->flags & ENTITY_INTERACTIVE &&
                             secondary->flags & ENTITY_ITEM &&
                             secondary->grid_x == entity->grid_x &&
                             secondary->grid_y == entity->grid_y )
                        {
                            state->player_score += secondary->score;

                            Rect blitrect;
                            blitrect.x = 0;
                            blitrect.y = entity->texture_rect.y - SANDWICH_STACK_OFFSET;
                            blitrect.w = entity->texture_rect.w;
                            blitrect.h = entity->texture_rect.w;

                            entity->texture_rect.y -= SANDWICH_STACK_OFFSET;
                            entity->texture_rect.h += SANDWICH_STACK_OFFSET;

                            if ( secondary->type == ItemCrecent )
                            {
                                state->start_next_level = 1;

                                entity->grid_x = -7;
                                entity->grid_y = 0;

                                Entity* tmpplayer = PUSH_STRUCT_INDEX( Entity, state->pool, state->transient_section );
                                memcpy( tmpplayer, entity, sizeof( Entity ));
                                ClearSectionIndex( state->pool, state->entity_section );
                                state->numentities = 0;
                                state->free_entity = NULL;
                                state->camera_follow = AddBlank( state, 0, 0 );
                                GenerateLevel( state );
                                Entity* new = GetFreeEntity( state );
                                memcpy( new, tmpplayer, sizeof( Entity ));
                                ClearSectionIndex( state->pool, state->transient_section );
                                AddWitch( state, 0, 0 );
                            }
                            else
                            {
                                ImageBlitScaled( secondary->texture, entity->texture, NULL, &blitrect, AlphaBinary );
                            }
                            DestroyEntity( state, secondary );
                        }
                        else if ( entity->flags & ENTITY_PLAYERCONTROLLED &&
                                  secondary->flags & ENTITY_EVIL &&
                                  secondary->grid_x == entity->grid_x &&
                                  secondary->grid_y == entity->grid_y )
                        {
                            //Beep( 400, 500 );
                            //Beep( 200, 1000 );
                            state->game_over = 1;
                        }
                    }
                }
            }
            if ( entity->dy == 0.0f && entity->dx == 0.0f )
            {
        #define MOVEMENT_SPEED 1.0f

                if ( entity->flags & ENTITY_PLAYERCONTROLLED )
                {
                    if ( state->in[entity->controller].keydown[KeyUp] &&
                         !blocks[KeyUp] )
                    {
                        entity->dy = -MOVEMENT_SPEED;
                        state->in[entity->controller].keydown[KeyUp] = 0;
                    }
                    else if ( state->in[entity->controller].keydown[KeyDown] &&
                              !blocks[KeyDown] )
                    {
                        entity->dy = MOVEMENT_SPEED;
                        state->in[entity->controller].keydown[KeyDown] = 0;
                    }
                    else if( state->in[entity->controller].keydown[KeyLeft] &&
                             !blocks[KeyLeft] )
                    {
                        entity->dx = -MOVEMENT_SPEED;
                        state->in[entity->controller].keydown[KeyLeft] = 0;
                    }
                    else if (state->in[entity->controller].keydown[KeyRight] &&
                             !blocks[KeyRight] )
                    {
                        entity->dx = MOVEMENT_SPEED;
                        state->in[entity->controller].keydown[KeyRight] = 0;
                    }
                }
                else if ( entity->flags & ENTITY_EVIL )
                {
                    Entity* player = NULL;
                    for ( j = 0; j < state->numentities; j++ )
                    {
                        if ( state->entities[j].flags & ENTITY_ACTIVE &&
                             state->entities[j].flags & ENTITY_PLAYERCONTROLLED )
                        {
                            player = &state->entities[j];
                            break;
                        }
                    }

                    if ( !player )
                    {
                        continue;
                    }

                    if ( !player->dx && !player->dy)
                    {
                        continue;
                    }

                    int nhigh = 0;
                    int nlow = 0;
                    PlayerKeys high_priority[4];
                    PlayerKeys low_priority[4];
                    if ( player->grid_x > entity->grid_x )
                    {
                        if ( !blocks[KeyRight] )
                        {
                            high_priority[nhigh++] = KeyRight;
                        }

                        if ( !blocks[KeyLeft] )
                        {
                            low_priority[nlow++] = KeyLeft;
                        }
                    }
                    else if ( player->grid_x < entity->grid_x )
                    {
                        if ( !blocks[KeyLeft] )
                        {
                            high_priority[nhigh++] = KeyLeft;
                        }

                        if ( !blocks[KeyRight] )
                        {
                            low_priority[nlow++] = KeyRight;
                        }
                    }
                    else
                    {
                        if ( !blocks[KeyLeft] )
                        {
                            low_priority[nlow++] = KeyLeft;
                        }

                        if ( !blocks[KeyRight] )
                        {
                            low_priority[nlow++] = KeyRight;
                        }
                    }

                    if ( player->grid_y > entity->grid_y )
                    {
                        if ( !blocks[KeyDown] )
                        {
                            high_priority[nhigh++] = KeyDown;
                        }

                        if ( !blocks[KeyUp] )
                        {
                            low_priority[nlow++] = KeyUp;
                        }
                    }
                    else if ( player->grid_y < entity->grid_y )
                    {
                        if ( !blocks[KeyUp] )
                        {
                            high_priority[nhigh++] = KeyUp;
                        }

                        if ( !blocks[KeyDown] )
                        {
                            low_priority[nlow++] = KeyDown;
                        }
                    }
                    else
                    {
                        if ( !blocks[KeyUp] )
                        {
                            low_priority[nlow++] = KeyUp;
                        }

                        if ( !blocks[KeyDown] )
                        {
                            low_priority[nlow++] = KeyDown;
                        }
                    }

                    PlayerKeys choice;
                    if ( nhigh )
                    {
                        choice = high_priority[xorshift64star_uniform( nhigh )];
                    }
                    else
                    {
                        choice = low_priority[xorshift64star_uniform( nlow )];
                    }

                    switch ( choice )
                    {
                        case KeyUp:
                            entity->dy = -MOVEMENT_SPEED;
                        break;

                        case KeyDown:
                            entity->dy = MOVEMENT_SPEED;
                        break;

                        case KeyLeft:
                            entity->dx = -MOVEMENT_SPEED;
                        break;

                        case KeyRight:
                            entity->dx = MOVEMENT_SPEED;
                        break;

                        default:
                        break;
                    }
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
            dstentity.x = (relgrid_x + entity->off_x + entity->dx * elapsed - left_edge) * ppmw;
            dstentity.y = (relgrid_y + entity->off_y + entity->dy * elapsed - top_edge) * ppmh;
            dstentity.w = state->grid_m * ppmw;
            dstentity.h = state->grid_m * ppmh;

            if ( entity->flags & ENTITY_PLAYERCONTROLLED )
            {
                uint32_t newh = entity->texture_rect.h * dstentity.w / entity->texture_rect.w;
                dstentity.y -= newh - dstentity.h;
                dstentity.h = newh;
            }

            ImageBlitScaled( entity->texture, screen, &entity->texture_rect, &dstentity, AlphaBinary );
        }
    }
}
