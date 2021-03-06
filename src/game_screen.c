#include "game_state.h"
#include "player_ship.h"
#include "events.h"
#include "menu_screen.h"
#include "enemies.h"
#include "alien_explosion.h"
#include "barrier.h"

// Checa se o jogador esta alinhado na vertical
// ou horizontal com as barreiras
bool player_alligned_with_barrier(PlayerShip *player, main_barrier *pbarr, char axis) {
    for(int i=0; i<NUM_BARRIERS; i++) {
        if(pbarr->main_bar[i].live)
            switch(axis) {
                case 'v':
                    if(player->pos_y + player->sprite.height >= pbarr->main_bar[i].y &&
                        player->pos_y <= pbarr->main_bar[i].y + pbarr->main_bar[i].frameheight) {
                        return true;
                    }
                    break;
                case 'h':
                    if(player->pos_x + player->sprite.width >= pbarr->main_bar[i].x &&
                        player->pos_x <= pbarr->main_bar[i].x + pbarr->main_bar[i].framewidth) {
                        return true;
                    }
                    break;
            }
    }
    return false;
}

// Detecta se o proximo movimento do jogador fará com que
// ele colida com uma barreira
bool detect_player_collision_barrier(PlayerShip *player, main_barrier *pbarr, char player_movement) {
    for(int i=0; i<NUM_BARRIERS; i++) {
        if(pbarr->main_bar[i].live)
            switch(player_movement) {
                case 'u':
                    if(player_alligned_with_barrier(player, pbarr, 'h') &&
                       player->pos_y - PLAYER_SPEED <= pbarr->main_bar[i].y + pbarr->main_bar[i].frameheight &&
                       player->pos_y - PLAYER_SPEED + player->sprite.height >= pbarr->main_bar[i].y)
                        return true;
                    break;
                case 'd':
                    if(player_alligned_with_barrier(player, pbarr, 'h') &&
                       player->pos_y + player->sprite.height + PLAYER_SPEED >= pbarr->main_bar[i].y &&
                       player->pos_y + PLAYER_SPEED <= pbarr->main_bar[i].y + pbarr->main_bar[i].frameheight)
                        return true;
                    break;
                case 'r':
                    if(player_alligned_with_barrier(player, pbarr, 'v') &&
                       player->pos_x + player->sprite.width + PLAYER_SPEED >= pbarr->main_bar[i].x &&
                       player->pos_x + PLAYER_SPEED <= pbarr->main_bar[i].x + pbarr->main_bar[i].framewidth)
                        return true;
                    break;
                case 'l':
                    if(player_alligned_with_barrier(player, pbarr, 'v') &&
                       player->pos_x - PLAYER_SPEED <= pbarr->main_bar[i].x + pbarr->main_bar[i].framewidth &&
                       player->pos_x - PLAYER_SPEED + player->sprite.width >= pbarr->main_bar[i].x)
                        return true;
                    break;
            }
    }
    return false;
}

// Detecta a colisão entre um disparo dos aliens com a nave do jogador
void detect_alien_bullet_collision_player(PlayerShip *player, enemies *p_enemies , GameState *game) {
    for(int i=0; i<NUM_aBULLETS; i++) {
        if(p_enemies->alienShots.aBullet[i].live) {
            if(p_enemies->alienShots.aBullet[i].y + p_enemies->alienShots.aBullet[i].height >= player->pos_y &&
               p_enemies->alienShots.aBullet[i].y <= player->pos_y + player->sprite.height &&
               p_enemies->alienShots.aBullet[i].x + p_enemies->alienShots.aBullet[i].width >= player->pos_x &&
               p_enemies->alienShots.aBullet[i].x <= player->pos_x + player->sprite.width) {
                player->lives--;
                p_enemies->alienShots.aBullet[i].live = false;

                al_play_sample((*game).Audio.player_explosion, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                DEBUG_PRINT("Collision! Current lifes: %d...\n", player->lives);
            }
        }
    }
}

// Move o jogador, impedindo que a nave do jogador saia da tela ou atravesse uma barreira
void process_player_movement(GameState *game, PlayerShip *player, main_barrier *pbarr) {
    if(game->keys_pressed[UP] &&
       player->pos_y - PLAYER_SPEED > SCREEN_HEIGHT/1.4 &&
       !detect_player_collision_barrier(player, pbarr, 'u'))
            player->pos_y -= PLAYER_SPEED;
    if(game->keys_pressed[DOWN] &&
       player->pos_y + PLAYER_SPEED <= SCREEN_HEIGHT - player->sprite.height &&
       !detect_player_collision_barrier(player, pbarr, 'd'))
            player->pos_y += PLAYER_SPEED;
    if(game->keys_pressed[LEFT] &&
       player->pos_x - PLAYER_SPEED >= 0 &&
       !detect_player_collision_barrier(player, pbarr, 'l'))
            player->pos_x -= PLAYER_SPEED;
    if(game->keys_pressed[RIGHT] &&
       player->pos_x + PLAYER_SPEED < SCREEN_WIDTH - player->sprite.width &&
       !detect_player_collision_barrier(player, pbarr, 'r'))
            player->pos_x += PLAYER_SPEED;
}

// Controla os disparos do jogador
void process_player_firing(GameState *game, PlayerShip *player) {
    bool fire = false;
    if(player->lasers.alive > 0 && player->lasers.alive < PLAYER_MAX_LASER_COUNT) {
        int last_laser = player->lasers.alive - 1;
        int dist_laser = player->pos_y - player->lasers.fired[last_laser].pos_y;
        if(dist_laser > SCREEN_HEIGHT / PLAYER_FIRING_SPEED) {
            fire = true;
        }
    } else if(player->lasers.alive == 0) {
        fire = true;
    }
    // Caso espaço esteja pressionado e o ultimo disparo do jogador
    // tenha uma distância mínima, adiciona um novo disparo
    if(game->keys_pressed[SPACE] && fire) {
        add_player_laser_fired(player);
        game->draw = true;
        al_play_sample((*game).Audio.player_shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

}

// Atualiza a tela do jogo
void update_game_screen(PlayerShip *player, enemies *p_enemies, main_barrier *Pbarr, GameMenu *menu, GameState *game) {
    // DEBUG_PRINT("Updating game screen!\n");
    al_clear_to_color(al_map_rgb(0, 0, 0));

    draw_player_ship(player);
    draw_player_lasers(player);

    drawBarrier(Pbarr);
    draw_explosion_barrier(Pbarr);

    draw_alien1(p_enemies);
    draw_alien2(p_enemies);
    draw_alien3(p_enemies);

    draw_explosion_alien1(p_enemies);
    draw_explosion_alien2(p_enemies);
    draw_explosion_alien3(p_enemies);

    draw_aBullet(p_enemies);

    draw_player_lives(player);

    al_draw_textf(game->font_score, al_map_rgb(255,255,255),
                                    762,
                                    5,
                                    ALLEGRO_ALIGN_CENTRE ,
                                    "SCORE: %d", player->cont_score);

    //desenha o esc menu quando necessario :
    if(!(*menu).Esc_menu.back_option_selected){ // var que so permite desenhar o esc menu quando esc for apertado.
        if((*menu).Esc_menu.current_esc_bitmap == 0){
            al_draw_bitmap((*menu).Esc_menu.esc_img_1, 411, 249, 0);
        }else if((*menu).Esc_menu.current_esc_bitmap == 1){
            al_draw_bitmap((*menu).Esc_menu.esc_img_2, 411, 249, 0);
        }
    }

    //desenha a tela de fim de jogo quando necessario:
    if((*menu).Endgame_menu.start_v){

        if((*menu).Endgame_menu.current_option_bitmap == 0){
            al_draw_bitmap((*menu).Endgame_menu.victory_img_1, 0, 0, 0);
            al_draw_textf(game->font_score, al_map_rgb(255,255,255),
                                            585, //Posicao X da tela onde vai ser printado o score
                                            305, //Posicao Y da tela onde vai ser printado o score
                                            ALLEGRO_ALIGN_CENTRE,
                                            "%d", player->cont_score);
        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
            al_draw_bitmap((*menu).Endgame_menu.victory_img_2, 0, 0, 0);
            al_draw_textf(game->font_score, al_map_rgb(255,255,255),
                                            585, //Posicao X da tela onde vai ser printado o score
                                            305, //Posicao Y da tela onde vai ser printado o score
                                            ALLEGRO_ALIGN_CENTRE,
                                            "%d", player->cont_score);
        }
    }else if((*menu).Endgame_menu.start_d){


        if((*menu).Endgame_menu.current_option_bitmap == 0){
            al_draw_bitmap((*menu).Endgame_menu.defeat_img_1, 0, 0, 0);
            al_draw_textf(game->font_score, al_map_rgb(255,255,255),
                                            585, //Posicao X da tela onde vai ser printado o score
                                            305, //Posicao Y da tela onde vai ser printado o score
                                            ALLEGRO_ALIGN_CENTRE,
                                            "%d", player->cont_score);
        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
            al_draw_bitmap((*menu).Endgame_menu.defeat_img_2, 0, 0, 0);
            al_draw_textf(game->font_score, al_map_rgb(255,255,255),
                                            585, //Posicao X da tela onde vai ser printado o score
                                            305, //Posicao Y da tela onde vai ser printado o score
                                            ALLEGRO_ALIGN_CENTRE,
                                            "%d", player->cont_score);
        }


    }

    al_flip_display();


}


// Modifica a posição dos sprites na tela
void process_game_events(GameState *game, GameMenu *menu, PlayerShip *player, enemies *p_enemies, main_barrier *Pbarr) {


    ALLEGRO_EVENT event;
    al_wait_for_event(game->event_queue, &event);

    if(event.type == ALLEGRO_EVENT_TIMER) {
        process_player_movement(game, player, Pbarr);
        process_player_firing(game, player);

        update_player_lasers(player);

        detect_alien_bullet_collision_player(player, p_enemies, game);



        detectBulletCollision_alien1(p_enemies, player, game);

        detectBulletCollision_alien2(p_enemies, player, game);

        detectBulletCollision_alien3(p_enemies, player, game);


        update_explosion_alien1(p_enemies);

        update_explosion_alien2(p_enemies);

        update_explosion_alien3(p_enemies);



        //barreira:

            colision_Alien_shot_barrier(Pbarr, p_enemies, game);

            colision_Player_shot_barrier(player, Pbarr, game);


            lifeBarrier(Pbarr, game);

            update_explosion_barrier(Pbarr);


        //Movimento dos aliens{


            limitXmoveY_alien1(p_enemies);

            limitXmoveY_alien2(p_enemies);

            limitXmoveY_alien3(p_enemies);



            moveX_alien1(p_enemies);

            moveX_alien2(p_enemies);

            moveX_alien3(p_enemies);



            updateSprite_alien1(p_enemies);

            updateSprite_alien2(p_enemies);

            updateSprite_alien3(p_enemies);
        //}


        //Disparo dos aliens{


            //delay para impedir que os aliens disparem muito rapido(a cada estouro do timer):
            if(++(*p_enemies).alienShots.alien_shot_delay >= 30){

                choose_shooter_alien(p_enemies);

                fire_aBullet(p_enemies);

                al_play_sample((*game).Audio.alien_shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                (*p_enemies).alienShots.alien_shot_delay = 0;
            }

            update_aBullet(p_enemies);

        //}

        //end-game logic:
            //victory:
        if((*p_enemies).aliens_defeated >= 50 && (*player).lives > 0){

            //desliga o audio de fundo do jogo:
            al_set_audio_stream_playing((*game).Audio.ingame_background, false);

            al_play_sample((*game).Audio.victory_song, 3.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

            (*menu).Endgame_menu.start_v = true;

            (*menu).Endgame_menu.current_option_bitmap = 0;
            update_game_screen(player, p_enemies, Pbarr, menu,game);

            bool victory_menu_option_selected = false;

            while(!victory_menu_option_selected){
                ALLEGRO_EVENT event_v;
                al_wait_for_event(game->event_queue, &event_v);
                process_events(game, &event_v);

                if(event_v.type == ALLEGRO_EVENT_KEY_DOWN){

                    if(game->keys_pressed[DOWN]){
                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            (*menu).Endgame_menu.current_option_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            (*menu).Endgame_menu.current_option_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }

                    }

                    if(game->keys_pressed[UP]){
                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            (*menu).Endgame_menu.current_option_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            (*menu).Endgame_menu.current_option_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }
                    }

                    if(game->keys_pressed[ENTER]){
                        victory_menu_option_selected = true;

                        al_play_sample((*game).Audio.selecting_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            player->lives = 0;
                            game->current_screen = GAME_SCREEN;
                            (*menu).Endgame_menu.ignore_main_menu = true;
                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            player->lives = 0;
                            game->current_screen = MENU_SCREEN;
                        }

                    }

                }

            }

            (*menu).Endgame_menu.start_v = false;
            //defeat:
        }else if((*p_enemies).aliens_defeated < 50 && (*player).lives <= 0){

            //desliga o audio de fundo do jogo:
            al_set_audio_stream_playing((*game).Audio.ingame_background, false);

            al_play_sample((*game).Audio.defeat_song, 3.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

            (*menu).Endgame_menu.start_d = true;

            (*menu).Endgame_menu.current_option_bitmap = 0;
             update_game_screen(player, p_enemies, Pbarr, menu, game);

            bool defeat_menu_option_selected = false;
            while(!defeat_menu_option_selected){

                ALLEGRO_EVENT event_d;
                al_wait_for_event(game->event_queue, &event_d);
                process_events(game, &event_d);

                if(event_d.type == ALLEGRO_EVENT_KEY_DOWN){

                    if(game->keys_pressed[DOWN]){

                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            (*menu).Endgame_menu.current_option_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            (*menu).Endgame_menu.current_option_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }

                    }

                    if(game->keys_pressed[UP]){

                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            (*menu).Endgame_menu.current_option_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            (*menu).Endgame_menu.current_option_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }
                    }

                    if(game->keys_pressed[ENTER]){
                        defeat_menu_option_selected = true;

                        al_play_sample((*game).Audio.selecting_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

                        if((*menu).Endgame_menu.current_option_bitmap == 0){
                            player->lives = 0;
                            game->current_screen = GAME_SCREEN;
                            (*menu).Endgame_menu.ignore_main_menu = true;

                        }else if((*menu).Endgame_menu.current_option_bitmap == 1){
                            player->lives = 0;
                            game->current_screen = MENU_SCREEN;
                        }

                    }

                }


            }

            (*menu).Endgame_menu.start_d = false;
        }

        game->draw = true;
    } else {
        process_events(game, &event);
        (*menu).Esc_menu.back_option_selected = true;       //indica quando não queremos printar o menu 'esc' dentro do jogo
        if(game->keys_pressed[ESC]) {

            al_play_sample((*game).Audio.esc_midgame, 3.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

            (*menu).Esc_menu.back_option_selected = false;
            (*menu).Esc_menu.current_esc_bitmap = 0;       // indica qual das duas imagens do menu esc esta sendo
                                                          //printada atualmente na tela;
            update_game_screen(player, p_enemies, Pbarr, menu, game);
            bool esc_menu_option_selected = false;  //var para verificar se alguma das opcoes do
                                                   // esc-menu ja foi selecionada.

            while(!esc_menu_option_selected){
                ALLEGRO_EVENT Event;
                al_wait_for_event(game->event_queue, &Event);
                process_events(game, &Event);

                if(Event.type == ALLEGRO_EVENT_KEY_DOWN){

                    if(game->keys_pressed[DOWN]){
                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        if((*menu).Esc_menu.current_esc_bitmap == 0){
                            (*menu).Esc_menu.current_esc_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);

                        }else if((*menu).Esc_menu.current_esc_bitmap == 1){
                            (*menu).Esc_menu.current_esc_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }
                    }

                    if(game->keys_pressed[UP]){
                        al_play_sample((*game).Audio.changing_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        if((*menu).Esc_menu.current_esc_bitmap == 0){
                            (*menu).Esc_menu.current_esc_bitmap = 1;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }else if ((*menu).Esc_menu.current_esc_bitmap == 1){
                            (*menu).Esc_menu.current_esc_bitmap = 0;
                            update_game_screen(player, p_enemies, Pbarr, menu, game);
                        }
                    }

                    if(game->keys_pressed[ENTER]){
                        esc_menu_option_selected = true;

                        if((*menu).Esc_menu.current_esc_bitmap == 0){
                            al_play_sample((*game).Audio.selecting_option, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            player->lives = 0;
                            game->current_screen = MENU_SCREEN;
                            //desliga o audio de fundo do jogo:
                            al_set_audio_stream_playing((*game).Audio.ingame_background, false);
                        }else if((*menu).Esc_menu.current_esc_bitmap == 1){
                            al_play_sample((*game).Audio.back_option, 3.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            (*menu).Esc_menu.back_option_selected = true;
                        }
                    }

                }


            }
        }
    }

    if(game->draw && al_is_event_queue_empty(game->event_queue) && (*menu).Esc_menu.back_option_selected) {
        game->draw = false;
        update_game_screen(player, p_enemies, Pbarr, menu, game);
    }
}
