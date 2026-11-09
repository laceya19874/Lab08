#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace sfp;

// Loads a texture from a file
void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}

// Calculates width and height
Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    RenderWindow window(VideoMode(800, 600), "Duck Hunter");
    World world(Vector2f(0, 0));
    int score(0);
    int arrows(5);


    SoundBuffer popBuffer;
    if (!popBuffer.loadFromFile("balloonpop.ogg")) {
        cout << "could not load balloonpop.ogg" << endl;
        exit(5);
    }
    Sound popSound;
    popSound.setBuffer(popBuffer);

    Music music;
    if (!music.openFromFile("circus.ogg")) {
        cout << "Failed to load circus.ogg" << endl;
        exit(6);
    }
    music.play();

    // Load the crossbow texture
    PhysicsSprite& crossBow = *new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400, 600 - (sz.y / 2)));

    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);



    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);

    // Despawn ducks wall
    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(850, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    Texture duckTex;
    LoadTex(duckTex, "images/Duck.png");
    PhysicsShapeList<PhysicsSprite> ducks;

    top.onCollision = [&drawingArrow, &world, &arrow]
    (PhysicsBodyCollisionResult result) {
        if (result.object2 == arrow) {
            drawingArrow = false;
            world.RemovePhysicsBody(arrow);
        }
        };

    // Loading the font
    Font fnt;
    if (!fnt.loadFromFile("arial.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }

    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

    // Spawn intervals
    long duckSpawnTimer = 0;
    const long SPAWN_INTERVAL = 800; // Spawns a new duck every 0.8 seconds

    // Main gameplay loop
    while ((arrows > 0) || drawingArrow) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return 0;
            }
        }

        // Calculate how many milliseconds have passed since the last frame
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();

        // Update physics and render the frame roughly every 10 milliseconds (~100 FPS)
        if (deltaMS > 9) {
            lastTime = currentTime;
            world.UpdatePhysics(deltaMS);

            // Creates a new duck when the timer reaches zero
            duckSpawnTimer -= deltaMS;
            if (duckSpawnTimer <= 0) {
                duckSpawnTimer = SPAWN_INTERVAL;

                // Creates new duck
                PhysicsSprite& duck = ducks.Create();
                duck.setTexture(duckTex);
                Vector2f duckSz = duck.getSize();

                // Positions the duck off screen
                duck.setCenter(Vector2f(-duckSz.x / 2, 20 + (duckSz.y / 2)));
                duck.setVelocity(Vector2f(0.40, 0));
                world.AddPhysicsBody(duck);

                // Duck collision when it hits an arrow or the right wall
                duck.onCollision = [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &popSound, &right]
                (PhysicsBodyCollisionResult result) {
                    if (result.object2 == arrow) {
                        popSound.play();
                        drawingArrow = false;
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                        score += 10;
                    }
                    else if (result.object2 == right) {
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                    }
                    };
            }

            if (Keyboard::isKeyPressed(Keyboard::Space) && !drawingArrow) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);
            }

            // Clear the previous frame from the window
            window.clear();

            // Draw the arrow if it is currently flying
            if (drawingArrow) {
                window.draw(arrow);
            }

            // Clean up any queued duck removals and draw all remaining active ducks
            ducks.DoRemovals();
            for (PhysicsShape& duck : ducks) {
                window.draw((PhysicsSprite&)duck);
            }

            // Draw the stationary crossbow
            window.draw(crossBow);

            // Draw current score in the bottom-right corner
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            Vector2f scoreSz = GetTextSize(scoreText);
            scoreText.setPosition(Vector2f(790 - scoreSz.x, 590 - scoreSz.y - 10));
            window.draw(scoreText);

            // Draw the remaining arrow count in the bottom-left corner
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            Vector2f arrowSz = GetTextSize(arrowCountText);
            arrowCountText.setPosition(Vector2f(10, 590 - arrowSz.y - 10));
            window.draw(arrowCountText);

            // Display everything drawn on this frame to the screen
            window.display();
        }
    }

    // Re-render the final gameplay state
    window.clear();
    ducks.DoRemovals();
    for (PhysicsShape& duck : ducks) {
        window.draw((PhysicsSprite&)duck);
    }
    window.draw(crossBow);

    // Draw the final
    Text scoreText;
    scoreText.setString(to_string(score));
    scoreText.setFont(fnt);
    Vector2f scoreSz = GetTextSize(scoreText);
    scoreText.setPosition(Vector2f(790 - scoreSz.x, 590 - scoreSz.y - 10));
    window.draw(scoreText);

    // Draw the final arrow count
    Text arrowCountText;
    arrowCountText.setString(to_string(arrows));
    arrowCountText.setFont(fnt);
    Vector2f arrowSz = GetTextSize(arrowCountText);
    arrowCountText.setPosition(Vector2f(10, 590 - arrowSz.y - 10));
    window.draw(arrowCountText);

    // Game over message
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();

    // Doesn't skip the end screen
    while (Keyboard::isKeyPressed(Keyboard::Space)) {
        sleep(milliseconds(10));
    }

    // Freeze on the Game Over frame, space to close the window
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Space)) {
            window.close();
        }
    }

    return 0;
}
