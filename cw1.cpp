// Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "loadShaders.hpp"

int screen = 1;
GLuint progID,vertexPosID,matrixID,shaderID,shadedVertexPosID,shadedVertexNormalID,modelID,viewID,projectionID,modelViewProjectionID,lightID,modelViewID,matrixID2;

//calculates values of sphere vertices and returns them in form of vector
std::vector<glm::vec3> SphereCoordinates(float r,float precision)
{
    std::vector<glm::vec3> points;
    float theta = 0, phi = 0, deltaTheta = M_PI / precision, deltaPhi = M_PI / precision;
    float x,y,z;

    for(theta =-0.5 * M_PI ; theta < (0.5*M_PI) ; theta += deltaTheta)
    {
        for(phi = 0; phi <(2*M_PI); phi += deltaPhi)
        {
            x = r * cos(theta) * cos(phi);
            y = r * cos(theta) * sin(phi);
            z = r * sin(theta);
            points.push_back(glm::vec3(x,y,z));

            x = r * cos(theta + deltaTheta) * cos(phi);
            y = r * cos(theta + deltaTheta) * sin (phi);
            z = r * sin(theta + deltaTheta);
            points.push_back(glm::vec3(x,y,z));

            x = r * cos(theta + deltaTheta) * cos(phi + deltaPhi);
            y = r * cos(theta + deltaTheta) * sin(phi + deltaPhi);
            z = r * sin(theta + deltaTheta);
            points.push_back(glm::vec3(x,y,z));

            x = r * cos(theta) * cos(phi + deltaPhi);
            y = r * cos(theta) * sin(phi + deltaPhi);
            z = r * sin(theta);
            points.push_back(glm::vec3(x,y,z));

        }
    }
    return points;
}

//calculates values of cone vertices and returns them in form of vector
std::vector<glm::vec3> ConeCoordinates(float r,float h)
{
    std::vector<glm::vec3> points;
    float x,y,z,cx,cy,theta = 0.25 * M_PI,di,dPhi,precision = 10;
    di = h / precision;
    dPhi = 2 * M_PI / 30;

    //bottom of cone
    points.push_back(glm::vec3(0,0,0));
    for(float phi = 0; phi <= 2 * M_PI; phi+=dPhi)
    {
        x = r * cos(theta);
        y = r * sin(theta);
        z = 0;

        cx = cos(phi);
        cy = sin(phi);
        points.push_back(glm::vec3(x * cx,y * cy,z));
        points.push_back(glm::vec3(0,0,0));
    }


    //cone body
    for(float phi = 0; phi <= 2 * M_PI; phi+=dPhi)
    {
        for(float i = 0; i < h; i+=di)
        {
            x = ((h - i)/h) * r * cos(theta);
            y = ((h - i)/h) * r * sin(theta);
            z = i;
            cx = cos(phi);
            cy = sin(phi);

            points.push_back(glm::vec3(x*cx,y*cy,z));
        }
    }

    for(float i = 0; i < h; i+=di)
    {
        x = ((h - i)/h) * r * cos(theta);
        y = ((h - i)/h) * r * sin(theta);
        z = i;

        for(float phi = 0; phi <= 2 * M_PI; phi+=dPhi)
        {
            cx = cos(phi);
            cy = sin(phi);

            points.push_back(glm::vec3(x*cx,y*cy,z));
        }
    }
    return points;
}

//given a vector of points, returns the normals of these points
std::vector<glm::vec3> NormalCoordinates(std::vector<glm::vec3>& givenPoints)
{
    std::vector<glm::vec3> normalPoints;
    int n = givenPoints.size();
    for(int i = 0; i < n; i++)
    {
        normalPoints.push_back(glm::normalize(givenPoints[i]));
    }
    return normalPoints;
}

//given a vector of points and another of its normals, returns coordinates to form the points with lines upwards at each
std::vector<glm::vec3> NormalLinesAddedCoordinates(std::vector<glm::vec3>& points,std::vector<glm::vec3>& normalPoints)
{
    std::vector<glm::vec3> pointsWithNormalLines;
    float scale = 0.1;
    int n = points.size();

    for(int i = 0; i < n; i++)
    {
        glm::vec3 point = points[i];
        glm::vec3 normalPoint = normalPoints[i];
        glm::vec3 addedNormalPoint = glm::vec3(point.x + (normalPoint.x * scale),point.y + (normalPoint.y * scale),point.z + (normalPoint.z * scale));
        pointsWithNormalLines.push_back(point);
        pointsWithNormalLines.push_back(addedNormalPoint);
        pointsWithNormalLines.push_back(point);
    }
    return pointsWithNormalLines;

}


//callback for GLFW - determines what will happen depending on what button is pressed
static void key_callback(GLFWwindow* window,int key, int scancode,int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window,GL_TRUE);
            break;
        case GLFW_KEY_A:
            screen = 1;
            break;
        case GLFW_KEY_B:
            screen = 2;
            break;
        case GLFW_KEY_C:
            screen = 3;
            break;
        case GLFW_KEY_D:
            screen = 4;
            break;
        case GLFW_KEY_E:
            screen = 5;
            break;
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window,GL_TRUE);
            break;
        default:
            break;

        }
    }
}

//binds buffer with given buffer and coordinates
void SetBuffer(GLuint& buffer,std::vector<glm::vec3> points)
{
    glGenBuffers(1,&buffer);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
        &points[0],
        GL_STATIC_DRAW
    );
}

//given a buffer and coordinates and the type of drawing, draws the shape
void DrawOnScreen(GLuint& buffer,std::vector<glm::vec3> points,GLenum type)
{
    glEnableVertexAttribArray(vertexPosID);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glVertexAttribPointer(vertexPosID,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    glDrawArrays(type,0,points.size());
    glDisableVertexAttribArray(vertexPosID);
}


int main()
{
    //initialise glfw
    if( !glfwInit() )
    {
        //Error
        fprintf(stderr,"Failed to initialise GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES,4); //4x antialiasing

    //Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024,768,"COMP3004 CW1", NULL, NULL);
    if(!window)
    {
        //error
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    //window is fine so make it current context
    glfwMakeContextCurrent(window);

    //initialise GLEW
    glewExperimental = true; // Needed in core profile
    int err = glewInit();
    if (GLEW_OK != err)
    {
        //Problem: glewInit failed, something is seriously wrong
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    //Everything works
    fprintf(stdout, "GL INFO : %s\n",glGetString(GL_VERSION));

    //setting key call back to check for escape key
    glfwSetKeyCallback(window,key_callback);

    //setting arrays
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    //sphere
    std::vector<glm::vec3> sphere = SphereCoordinates(1,40);
    GLuint sphereBuffer;
    SetBuffer(sphereBuffer,sphere);

    //cone
    GLuint coneBuffer;
    std::vector<glm::vec3> cone = ConeCoordinates(1,1.5);
    SetBuffer(coneBuffer,cone);

    //hedgehog
    GLuint normalsBuffer;
    GLuint hedgehogBuffer;
    std::vector<glm::vec3> sphereNormals = NormalCoordinates(sphere);
    std::vector<glm::vec3> hedgehog = NormalLinesAddedCoordinates(sphere,sphereNormals);
    SetBuffer(normalsBuffer,sphereNormals);
    SetBuffer(hedgehogBuffer,hedgehog);


    //Projection Matrix : 45° Field of view, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f,4.0f/3.0f,0.1f,100.0f);
    //Camera Matrix
    glm::mat4 View = glm::lookAt(
                         glm::vec3(4,3,3), //Camera is at 4,3,3, in World Space
                         glm::vec3(0,0,0), //and looks at origin
                         glm::vec3(0,1,0) //Head is up (set to 0,-1,0 to look upside-down
                     );

    //Model Matrix : an identity matrix(model wll be at origin
    glm::mat4 Model = glm::mat4(1.0f); //will change for each model

    //Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP = Projection * View * Model; //Matrix multiplication is other way round
    glm::mat4 MV = View * Model;


    //MVP for scene E
    glm::mat4 Projection2 = glm::perspective(45.0f,4.0f/3.0f,0.1f,100.0f);
    glm::mat4 View2 = glm::lookAt(glm::vec3(4,3,3),glm::vec3(0,0,0),glm::vec3(0,1,0));
    glm::mat4 Model2 = glm::mat4(1.0f);

    //vertex and fragment shaders
    //for wireframes
    progID = LoadShaders("simplevertexshader.vertexshader", "simplefragmentshader.fragmentshader");
    vertexPosID = glGetAttribLocation(progID, "a_vertex");
    matrixID = glGetUniformLocation(progID,"mvp");
    matrixID2 = glGetUniformLocation(progID,"mvp");

    //for shaded
    shaderID = LoadShaders("complexvertexshader.vertexshader","complexfragmentshader.fragmentshader");
    shadedVertexPosID = glGetAttribLocation(shaderID,"a_vertex");
    shadedVertexNormalID = glGetAttribLocation(shaderID,"a_normal");
    modelID = glGetUniformLocation(shaderID,"m");
    viewID = glGetUniformLocation(shaderID,"v");
    modelViewID = glGetUniformLocation(shaderID,"mv");
    projectionID = glGetUniformLocation(shaderID,"p");
    modelViewProjectionID = glGetUniformLocation(shaderID,"mvp");
    lightID = glGetUniformLocation(shaderID,"lightPosition_worldspace");

    //light falling - same as camera
    glm::vec3 lightPos = glm::vec3(4,3,3);

    float angle = 0;

    //clear screen and keep its colour black
    glClearColor(0,0,0,0);
    while( !glfwWindowShouldClose(window))  //while window has not been closed
    {
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);    //for shaded sphere
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //so colour remains what was set outside the loop

        //Choose which shaders will be used
        glUseProgram(progID);

        //assign uniform mvp in shaders
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
        switch(screen)
        {
        case 1:
            DrawOnScreen(sphereBuffer,sphere,GL_LINE_STRIP);
            break;
        case 2:
            DrawOnScreen(coneBuffer,cone,GL_LINE_STRIP);
            break;
        case 3:
            DrawOnScreen(hedgehogBuffer,hedgehog,GL_LINE_STRIP);
            break;
        case 4:
            //use the program with shader vertex and fragment shaders
            glUseProgram(shaderID);

            glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
            glUniformMatrix4fv(viewID,1,GL_FALSE,&View[0][0]);
            glUniformMatrix4fv(modelViewID,1,GL_FALSE,&MV[0][0]);
            glUniformMatrix4fv(projectionID,1,GL_FALSE,&Projection[0][0]);
            glUniformMatrix4fv(modelViewProjectionID, 1, GL_FALSE, &MVP[0][0]);
            glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);

            glEnableVertexAttribArray(shadedVertexPosID);
            glBindBuffer(GL_ARRAY_BUFFER,sphereBuffer);
            glVertexAttribPointer(shadedVertexPosID,3,GL_FLOAT,GL_FALSE,0,(void *)0);

            glEnableVertexAttribArray(shadedVertexNormalID);
            glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
            glVertexAttribPointer(shadedVertexNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

            glDrawArrays(GL_QUADS,0,sphere.size());
            glDisableVertexAttribArray(shadedVertexPosID);
            break;
        case 5:
            //cones and their paths

            angle = 0.2 * 2 * M_PI *(float) glfwGetTime();
            if(angle >=  2 * M_PI)
            {
                angle -=  2 * M_PI;
            }

            Model2 = glm::scale(glm::mat4(1.f), glm::vec3(0.75f));
            Model2 = glm::translate(Model2,glm::vec3(-5,glm::cos(angle), 0));
            Model2 = glm::rotate(Model2, (angle * 20), glm::vec3(1, 0, 0));
            MVP = Projection2 * View2 * Model2;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            DrawOnScreen(coneBuffer,cone,GL_LINE_STRIP);

            Model2 = glm::scale(glm::mat4(1.f), glm::vec3(-0.75f));
            Model2 = glm::translate(Model2,glm::vec3(glm::sin(angle),0, 0));
            Model2 = glm::rotate(Model2, (angle * 20), glm::vec3(0, 0, 1));
            MVP = Projection2 * View2 * Model2;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            DrawOnScreen(coneBuffer,cone,GL_LINE_STRIP);

            //spheres and their paths
            Model2 = glm::scale(glm::mat4(2.f), glm::vec3(0.35f));
            Model2 = glm::translate(Model2,glm::vec3(glm::sin(angle),-5, 0));
            Model2 = glm::rotate(Model2, (angle * 20), glm::vec3(0, 1, 1));
            MVP = Projection2 * View2 * Model2;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            DrawOnScreen(sphereBuffer,sphere,GL_LINE_STRIP);

            Model2 = glm::scale(glm::mat4(2.f), glm::vec3(0.5f));
            Model2 = glm::translate(Model2,glm::vec3(4,glm::sin(angle), 0));
            Model2 = glm::rotate(Model2, (angle * 20), glm::vec3(1, 1, 0));
            MVP = Projection2 * View2 * Model2;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            DrawOnScreen(sphereBuffer,sphere,GL_LINE_STRIP);
            break;
        default:
            break;
        }

        //rotating against angle
        angle = 20.0f  *(float) glfwGetTime();
        if (angle >= 360)
            angle -= 360;
        Model = glm::rotate(glm::mat4(1.f), angle, glm::vec3(1, 1, 0));
        MVP = Projection * View * Model;
        MV = View * Model;

        //keep running
        glfwSwapBuffers(window);

        //check for events
        glfwPollEvents();
    }

    glDeleteBuffers(1, &sphereBuffer);
    glDeleteBuffers(1, &normalsBuffer);
    glDeleteBuffers(1, &hedgehogBuffer);
    glDeleteBuffers(1, &coneBuffer);
    glDeleteProgram(progID);
    glDeleteProgram(shaderID);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
