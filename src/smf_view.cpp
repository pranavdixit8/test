/**
	@file 		smf_view.cpp
	@author 	Ravi Agrawal (ravia@sfu.ca)
	@date		November 7, 2017
	@version 	1.0

	@brief		CMPT-764: Assignment 2, Implements SMF viewer.
*/

#include <gl.h>

#include <GL/glut.h>
#include <GL/glui.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <ctime>

#include <parser.h>
#include <part.h>
#include <group.h>
#include <scorer.h>
#include <cgal.h>
#include "RelationContainer.h"
#include "relation.h"
#include "game.h"
#include <kmean.h>

#define WIDTH 1200
#define HEIGHT 800

using std::runtime_error;
using std::shared_ptr;
using std::array;

enum Buttons {ROTATION, OPEN, OPEN_DIR, START_GAME, SAVE, QUIT, CHAIR_A, CHAIR_B, SWAP_LEGS, SWAP_BACK, SWAP_SEAT, GAME_SELECT_A, GAME_SELECT_B, GAME_SELECT_BOTH, GAME_SLEECT_NONE, GAME_SAVE_A, GAME_SAVE_B};

Game *game;

vector<PartBase*> chairs;
PartBase *chair;

PartBase *chairA;
PartBase *chairB;

Part *part;
Part *part2;

PartBase *g1;
PartBase *g2;

float xy_aspect;
int last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;

// Live Variables
int main_window;
int displayType = FLAT_SHADED;
float scale = 2.5;
float view_rotate[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float chairA_rotate[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float chairB_rotate[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float obj_pos[] = {0.0, 0.0, -5.0};

void updateGLUI (vector<PartBase*>);
void game_GLUI ();

// GLUT idle function
void glutIdle (void) {
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);

	glutPostRedisplay();
}

// GLUT reshape function
void glutReshape (int x, int y) {	
	xy_aspect = (float) x / (float) y;
	GLUI_Master.auto_set_viewport();

	glutPostRedisplay();
}

void displayAxes () {
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 1.0);
	glEnd();
}

// Display mesh function
void displayMesh (void) {	

	// for (PartBase *chair : chairs) {
	// 	chair->render((DisplayType) displayType);
	// }

	// if (chair == NULL)
	// 	return;
	glPushMatrix();
	glTranslatef(-1,0,0);
	glMultMatrixf(chairA_rotate);
	glTranslatef(0,0,-0.5);
	if (chairA != NULL)
		chairA->render((DisplayType) displayType);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1,0,0);
	glMultMatrixf(chairB_rotate);
	glTranslatef(0,0,-0.5);
	if (chairB != NULL)
		chairB->render((DisplayType) displayType);
	glPopMatrix();
}

// GLUT display function
void glutDisplay (void) {
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xy_aspect*0.08, xy_aspect*0.08, -0.08, 0.08, 0.1, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Handle transformations
	glTranslatef(obj_pos[0], obj_pos[1], obj_pos[2]);
	glScalef(scale, scale, scale);
	glMultMatrixf(view_rotate);

	glColor3f(1.0, 1.0, 0.0);

	displayAxes();
	displayMesh();

	glutSwapBuffers();
}

// Execute shell commands
string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

// GLUI control callback
void control_cb(int control) {
	switch (control) {
		case OPEN: {
			string inputFilePath;
			inputFilePath = exec("zenity --file-selection --file-filter='3D Object files (smf,obj) | *.smf *.obj' --title=\"Select a SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			inputFilePath = inputFilePath.substr(0, inputFilePath.size() - 1);
			if (inputFilePath.size() != 0) {
				part = Part::initPart("Test", inputFilePath);
			}
			break;
		}

		case OPEN_DIR: {
			string folderPath;
			folderPath = exec("zenity --file-selection --directory --title=\"Select a Directory\" 2>/dev/null");
			// Remove the newline character at the end
			folderPath = folderPath.substr(0, folderPath.size() - 1);

			// Load Files
			chairs = loadFiles(folderPath);
			updateGLUI(chairs);

			for(int i =0 ; i < chairs.size(); i++){

				std::cout << "\n\nCheck if Chair "<<i<<" plausible? " << std::endl;
				int isPlausibleChair = isPlausible(chairs[i]);
				std::cout << "Plausible:\t" << isPlausibleChair<< std::endl<<std::endl;
			}
			
			getLabels(chairs);



			break;
		}

		case START_GAME: {
			string fPath;
			fPath = exec("zenity --file-selection --directory --title=\"Select a Directory\" 2>/dev/null");
			// Remove the newline character at the end
			fPath = fPath.substr(0, fPath.size() - 1);
			if (fPath.size() != 0) {
				// Load Files
				game = Game::initGame(fPath);
				game->run();
				game_GLUI();
			}
			break;
		}

		case SAVE: {
			string saveFilePath;
			saveFilePath = exec("zenity --file-selection --save --confirm-overwrite --title=\"Save SMF file\" 2>/dev/null");
			// Remove the newline character at the end
			saveFilePath = saveFilePath.substr(0, saveFilePath.size() - 1);
			if (saveFilePath.size() != 0);
			break;
		}
	}
};

GLUI* glui;
GLUI_Panel *chairsPanel;
GLUI_Listbox *chairsBoxA;
GLUI_Listbox *chairsBoxB;
GLUI_Rotation *chairRotA;
GLUI_Rotation *chairRotB;
int chairAIndex;
int chairBIndex;

void swap (PartBase *chairA, PartBase *chairB, string GroupName) {
	g1 = chairA->getMember(GroupName);
	g2 = chairB->getMember(GroupName);

	Group *group1 = (Group*) g1;
	Group *group2 = (Group*) g2; 

	vector<pair<PartBase*, PartBase*>> pairs; 
	for (PartBase *p1: group1->members) {
		for (PartBase *p2: group2->members) {
			Part *part1 = (Part*) p1;
			Part *part2 = (Part*) p2;
			float hausdorffDistance = getHausdorffDistance(part1->mesh, part2->mesh);
			if (hausdorffDistance < 0.25) {
				pairs.push_back(make_pair(p1, p2));
			}
		}		
	}

	for (pair<PartBase*, PartBase*> pair : pairs) {
		Part *part1 = (Part*) pair.first->make_copy();
		Part *part2 = (Part*) pair.second->make_copy();
		part2->transformTo(part1);
		part1->transformTo(part2);
		part1 = (Part*) part1->make_copy();
		part2 = (Part*) part2->make_copy();
		chairA->getMember(GroupName)->setMember(part1->label, part2);
		chairB->getMember(GroupName)->setMember(part2->label, part1);
	}
}

// GLUI chair callback
void chair_cb (int control) {
	if (control == CHAIR_A) {
		chairA = chairs[chairAIndex];
	} else if (control == CHAIR_B) {
		chairB = chairs[chairBIndex];
	} else if (control == SWAP_LEGS) {
		PartBase *legA = chairA->getMember("Leg_Group");
		PartBase *legB = chairB->getMember("Leg_Group");
		swap(chairA, chairB, "Leg_Group");

		// chairA->setMember("Leg_Group", legB);
		// chairB->setMember("Leg_Group", legA);
	} else if (control == SWAP_BACK) {
		PartBase *backA = chairA->getMember("Back_Group");
		PartBase *backB = chairB->getMember("Back_Group");
		swap(chairA, chairB, "Back_Group");

		// chairA->setMember("Back_Group", backB);
		// chairB->setMember("Back_Group", backA);
	} else if (control == SWAP_SEAT) {
		PartBase *seatA = chairA->getMember("Seat_Group");
		PartBase *seatB = chairB->getMember("Seat_Group");
		swap(chairA, chairB, "Seat_Group");

		// chairA->setMember("Seat_Group", seatB);
		// chairB->setMember("Seat_Group", seatA);
	}
}

void updateGLUI (vector<PartBase*> chairs) {
	chairsPanel = glui->add_panel("Chairs");

	chairsBoxA = new GLUI_Listbox(chairsPanel, "Chair A:", &chairAIndex, CHAIR_A, chair_cb);
	chairRotA = glui->add_rotation_to_panel(chairsPanel, "Rotate Chair A", chairA_rotate);
	chairRotA->set_spin(1.0);

	chairsBoxB = new GLUI_Listbox(chairsPanel, "Chair B:", &chairBIndex, CHAIR_B, chair_cb);
	chairRotB = glui->add_rotation_to_panel(chairsPanel, "Rotate Chair B", chairB_rotate);
	chairRotB->set_spin(1.0);

	glui->add_button_to_panel(chairsPanel, "Swap Legs", SWAP_LEGS, chair_cb);
	glui->add_button_to_panel(chairsPanel, "Swap Back", SWAP_BACK, chair_cb);
	glui->add_button_to_panel(chairsPanel, "Swap Seat", SWAP_SEAT, chair_cb);

	for (int i = 0; i < chairs.size(); i++) {
		PartBase *chair = chairs[i];
		chairsBoxA->add_item(i, (chair->label).c_str());
		chairsBoxB->add_item(i, (chair->label).c_str());
	}

	chairA = chairs[0];
	chairB = chairs[0];
};

// Setup GLUI
void setupGlui () {
	// Initialize GLUI subwindow
	glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	
	// Set main GFX window	
	glui->set_main_gfx_window(main_window);
	
	// Setup UI
	// Add Panel "Display Options"
	GLUI_Panel *displayOptionsPanel = glui->add_panel("Display Options");
	GLUI_Panel *transformationsPanel = glui->add_panel("Transformations");
	GLUI_Panel *controlsPanel = glui->add_panel("Controls");

	// Add Listbox
	GLUI_Listbox *listbox = new GLUI_Listbox(displayOptionsPanel, "Display Type:", &displayType);
	listbox->add_item(FLAT_SHADED, "Flat Shaded");
	listbox->add_item(SMOOTH_SHADED, "Smooth Shaded");
	listbox->add_item(WIREFRAME, "Wireframe");
	listbox->add_item(SHADED_WITH_EDGES, "Shaded with Edges");
	listbox->add_item(PRIMITIVES, "Primitives");

	// Add Scale Spinner
	GLUI_Spinner *scale_spinner = new GLUI_Spinner(transformationsPanel, "Scale:", &scale);
  	scale_spinner->set_float_limits(0.2f, 5.0);
  	scale_spinner->set_alignment(GLUI_ALIGN_RIGHT);
	
	// Add Rotation
	GLUI_Panel *rotation_panel = glui->add_panel_to_panel(transformationsPanel, "", GLUI_PANEL_NONE);
	GLUI_Rotation *view_rot = glui->add_rotation_to_panel(rotation_panel, "Rotate", view_rotate, ROTATION, control_cb);
	view_rot->set_spin(1.0);

	// Add Translation
	// GLUI_Panel *translation_panel = glui->add_panel_to_panel(transformationsPanel, "", GLUI_PANEL_NONE);
	// GLUI_Translation *trans_xy = glui->add_translation_to_panel(translation_panel, "Translate XY", GLUI_TRANSLATION_XY, obj_pos);
	// trans_xy->scale_factor = 0.1f;
	// GLUI_Translation *trans_z = glui->add_translation_to_panel(translation_panel, "Translate Z", GLUI_TRANSLATION_Z, &obj_pos[2]);
	// trans_z->scale_factor = 0.1f;

	// Add Buttons
	glui->add_button_to_panel(controlsPanel, "Open", OPEN, control_cb);
	glui->add_button_to_panel(controlsPanel, "Open Dir", OPEN_DIR, control_cb);
	glui->add_button_to_panel(controlsPanel, "Start Game", START_GAME, control_cb);
	glui->add_button_to_panel(controlsPanel, "Save", SAVE, control_cb);
	glui->add_button_to_panel(controlsPanel, "Quit", QUIT, (GLUI_Update_CB)exit);
};

int main(int argc, char* argv[]) {
	srand(time(NULL));


	if (argc > 1) {
		if (string(argv[1]) == string("train")) {
			string path = argv[2];

			string pathPositive = path + "/positive";
			string pathNegative = path + "/negative";

			trainAllModels(pathPositive, pathNegative);
		} else if (string(argv[1]) == string("predict")) {
			string path = argv[2];

			Mat img = imread(path.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
			int prediction;
			if (atoi(path.c_str()) % 3 == 1) {
				prediction = predict(img, FRONT);
			} else if (atoi(path.c_str()) % 3 == 2) {
				prediction = predict(img, SIDE);
			} else if (atoi(path.c_str()) % 3 == 0) {
				prediction = predict(img, TOP);
			}

			cout << "Prediction: " << prediction << endl;
		}
		return 0;
	}

	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	// Initialize Window
	main_window = glutCreateWindow("SMF View");

	// Register Callbacks
	glutDisplayFunc(glutDisplay);
	GLUI_Master.set_glutReshapeFunc(glutReshape);
	GLUI_Master.set_glutIdleFunc(glutIdle);
	
	// Setup Lights
	GLfloat light0_ambient[] = {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light0_diffuse[] = {0.6f, 0.6f, 1.0f, 1.0f};
	GLfloat light0_position[] = {1.0f, 1.0f, 1.0f, 0.0f};
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	// Setup Color
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Enable z-buffering
	glEnable(GL_DEPTH_TEST);

	// Setup GLUI
	setupGlui();

	// Start Main Loop
	glutMainLoop();
}






//----------------- GAME -----------------

string round_text = "";


// GLUI game callback
void game_cb (int control) {

	bool to_continue = false;

	if(control == GAME_SELECT_A)
		to_continue = game->do_select("A");
	else if(control == GAME_SELECT_B)
		to_continue = game->do_select("B");
	else if(control == GAME_SELECT_BOTH)
		to_continue = game->do_select("BOTH");	
	else if(control == GAME_SLEECT_NONE)
		to_continue = game->do_select("NONE");	
	else if(control == GAME_SAVE_A)
		game->do_save("A");
	else if(control == GAME_SAVE_B)
		game->do_save("B");


	round_text = game->current_round + "/" + game->round;

	if(to_continue)
	{
		chairA = game->getChairA();
		chairB = game->getChairB();
	} else {
		// todo: the game is done

	}
}



void game_GLUI () {
	chairsPanel = glui->add_panel("Game");
	
	//GLUI_StaticText
	// GLUI_StaticText s_text_1 = glui->add_statictext_to_panel(chairsPanel, "round: " + game->current_round + "/" + game->round );
	// GLUI_StaticText s_text_1 = glui->add_statictext_to_panel(chairsPanel, "selected new chairs: " + game->getNewChairsNum() );

	// todo; fix this
	//GLUI_EditText * height_text = glui->add_edittext_to_panel(cone_controls,"Height", GLUI_EDITTEXT_FLOAT, &height);
	round_text = game->current_round + "/" + game->round;
	GLUI_EditText *editText = glui->add_edittext_to_panel(chairsPanel, "Round: ", GLUI_EDITTEXT_TEXT, &round_text);
	 


	chairRotA = glui->add_rotation_to_panel(chairsPanel, "Rotate Chair A", chairA_rotate);
	chairRotA->set_spin(1.0);
	
	chairRotB = glui->add_rotation_to_panel(chairsPanel, "Rotate Chair B", chairB_rotate);
	chairRotB->set_spin(1.0);

	glui->add_button_to_panel(chairsPanel, "A", GAME_SELECT_A, game_cb);
	glui->add_button_to_panel(chairsPanel, "B", GAME_SELECT_B, game_cb);
	glui->add_button_to_panel(chairsPanel, "Both", GAME_SELECT_BOTH, game_cb);
	glui->add_button_to_panel(chairsPanel, "None", GAME_SLEECT_NONE, game_cb);

	glui->add_button_to_panel(chairsPanel, "Save A", GAME_SAVE_A, game_cb);
	glui->add_button_to_panel(chairsPanel, "Save B", GAME_SAVE_B, game_cb);
	
	chairA = game->getChairA();
	chairB = game->getChairB();	
};
