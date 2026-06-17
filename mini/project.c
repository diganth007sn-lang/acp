#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Shape Types
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Coordinates & dimensions
typedef struct {
    int x1, y1, x2, y2;
} LineData;

typedef struct {
    int x, y, w, h;
} RectData;

typedef struct {
    int cx, cy, r;
} CircleData;

typedef struct {
    int x1, y1, x2, y2, x3, y3;
} TriData;

// Double Linked List Node
typedef struct ShapeNode {
    int id;
    ShapeType type;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriData tri;
    } data;
    char draw_char; // Character used to draw this shape
    struct ShapeNode *prev;
    struct ShapeNode *next;
} ShapeNode;

// Global settings
char **canvas = NULL;
int canvas_width = 80;
int canvas_height = 24;
char bg_char = '_';

// Linked list management
ShapeNode *head = NULL;
ShapeNode *tail = NULL;
int next_id = 1;

// Initialize dynamic canvas memory
void init_canvas() {
    canvas = (char **)malloc(canvas_height * sizeof(char *));
    for (int i = 0; i < canvas_height; i++) {
        canvas[i] = (char *)malloc(canvas_width * sizeof(char));
    }
}

// Free canvas memory
void free_canvas() {
    if (canvas != NULL) {
        for (int i = 0; i < canvas_height; i++) {
            free(canvas[i]);
        }
        free(canvas);
        canvas = NULL;
    }
}

// Reset canvas with background char
void clear_canvas() {
    for (int y = 0; y < canvas_height; y++) {
        for (int x = 0; x < canvas_width; x++) {
            canvas[y][x] = bg_char;
        }
    }
}

// Bresenham's Line Algorithm
void draw_line(int x1, int y1, int x2, int y2, char c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x1 >= 0 && x1 < canvas_width && y1 >= 0 && y1 < canvas_height) {
            canvas[y1][x1] = c;
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw rectangle outline
void draw_rectangle(int x, int y, int w, int h, char c) {
    if (w <= 0 || h <= 0) return;
    draw_line(x, y, x + w - 1, y, c);                 // Top
    draw_line(x, y + h - 1, x + w - 1, y + h - 1, c); // Bottom
    draw_line(x, y, x, y + h - 1, c);                 // Left
    draw_line(x + w - 1, y, x + w - 1, y + h - 1, c); // Right
}

// Circle symmetric pixels plotter
void draw_circle_pixels(int xc, int yc, int x, int y, char c) {
    int xs[8] = { xc + x, xc - x, xc + x, xc - x, xc + y, xc - y, xc + y, xc - y };
    int ys[8] = { yc + y, yc + y, yc - y, yc - y, yc + x, yc + x, yc - x, yc - x };
    for (int i = 0; i < 8; i++) {
        int px = xs[i];
        int py = ys[i];
        if (px >= 0 && px < canvas_width && py >= 0 && py < canvas_height) {
            canvas[py][px] = c;
        }
    }
}

// Midpoint Circle Algorithm
void draw_circle(int xc, int yc, int r, char c) {
    if (r < 0) return;
    if (r == 0) {
        if (xc >= 0 && xc < canvas_width && yc >= 0 && yc < canvas_height) {
            canvas[yc][xc] = c;
        }
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    draw_circle_pixels(xc, yc, x, y, c);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_pixels(xc, yc, x, y, c);
    }
}

// Draw triangle outline
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char c) {
    draw_line(x1, y1, x2, y2, c);
    draw_line(x2, y2, x3, y3, c);
    draw_line(x3, y3, x1, y1, c);
}

// Add shape node to double linked list
void add_shape_node(ShapeNode *node) {
    node->next = NULL;
    if (head == NULL) {
        node->prev = NULL;
        head = node;
        tail = node;
    } else {
        node->prev = tail;
        tail->next = node;
        tail = node;
    }
}

// Free all shape nodes
void free_shapes_list() {
    ShapeNode *curr = head;
    while (curr != NULL) {
        ShapeNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
    tail = NULL;
}

// Find node by ID
ShapeNode* find_shape(int id) {
    ShapeNode *curr = head;
    while (curr != NULL) {
        if (curr->id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

// Delete shape node by ID
int delete_shape_node(int id) {
    ShapeNode *curr = find_shape(id);
    if (curr == NULL) return 0;

    if (curr->prev != NULL) {
        curr->prev->next = curr->next;
    } else {
        head = curr->next;
    }

    if (curr->next != NULL) {
        curr->next->prev = curr->prev;
    } else {
        tail = curr->prev;
    }

    free(curr);
    return 1;
}

// Reordering: Bring to Front (renders last, appears on top)
void bring_to_front(int id) {
    ShapeNode *curr = find_shape(id);
    if (curr == NULL || curr == tail) return;

    // Unlink
    if (curr->prev != NULL) {
        curr->prev->next = curr->next;
    } else {
        head = curr->next;
    }
    if (curr->next != NULL) {
        curr->next->prev = curr->prev;
    }

    // Insert at tail
    curr->prev = tail;
    curr->next = NULL;
    if (tail != NULL) {
        tail->next = curr;
    }
    tail = curr;
    if (head == NULL) {
        head = curr;
    }
}

// Reordering: Send to Back (renders first, appears underneath)
void send_to_back(int id) {
    ShapeNode *curr = find_shape(id);
    if (curr == NULL || curr == head) return;

    // Unlink
    if (curr->next != NULL) {
        curr->next->prev = curr->prev;
    } else {
        tail = curr->prev;
    }
    if (curr->prev != NULL) {
        curr->prev->next = curr->next;
    }

    // Insert at head
    curr->next = head;
    curr->prev = NULL;
    if (head != NULL) {
        head->prev = curr;
    }
    head = curr;
    if (tail == NULL) {
        tail = curr;
    }
}

// Render list of shapes onto canvas
void render_canvas() {
    clear_canvas();
    ShapeNode *curr = head;
    while (curr != NULL) {
        switch (curr->type) {
            case SHAPE_LINE:
                draw_line(curr->data.line.x1, curr->data.line.y1, curr->data.line.x2, curr->data.line.y2, curr->draw_char);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(curr->data.rect.x, curr->data.rect.y, curr->data.rect.w, curr->data.rect.h, curr->draw_char);
                break;
            case SHAPE_CIRCLE:
                draw_circle(curr->data.circle.cx, curr->data.circle.cy, curr->data.circle.r, curr->draw_char);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(curr->data.tri.x1, curr->data.tri.y1, curr->data.tri.x2, curr->data.tri.y2, curr->data.tri.x3, curr->data.tri.y3, curr->draw_char);
                break;
        }
        curr = curr->next;
    }
}

// Print canvas
void display_canvas() {
    render_canvas();
    
    // Top border
    printf(" +");
    for (int x = 0; x < canvas_width; x++) printf("-");
    printf("+\n");

    // Canvas contents
    for (int y = 0; y < canvas_height; y++) {
        printf(" |");
        for (int x = 0; x < canvas_width; x++) {
            printf("%c", canvas[y][x]);
        }
        printf("|\n");
    }

    // Bottom border
    printf(" +");
    for (int x = 0; x < canvas_width; x++) printf("-");
    printf("+\n");
}

// Print shapes in a detailed table
void list_shapes() {
    if (head == NULL) {
        printf("No shapes exist currently.\n");
        return;
    }
    printf("\n%-5s | %-12s | %-35s | %-9s\n", "ID", "Shape Type", "Dimensions/Coordinates", "Char");
    printf("--------------------------------------------------------------------\n");
    ShapeNode *curr = head;
    while (curr != NULL) {
        char coords[100];
        switch (curr->type) {
            case SHAPE_LINE:
                sprintf(coords, "(%d, %d) to (%d, %d)", curr->data.line.x1, curr->data.line.y1, curr->data.line.x2, curr->data.line.y2);
                printf("%-5d | %-12s | %-35s | '%c'\n", curr->id, "Line", coords, curr->draw_char);
                break;
            case SHAPE_RECTANGLE:
                sprintf(coords, "top-left (%d, %d), size %dx%d", curr->data.rect.x, curr->data.rect.y, curr->data.rect.w, curr->data.rect.h);
                printf("%-5d | %-12s | %-35s | '%c'\n", curr->id, "Rectangle", coords, curr->draw_char);
                break;
            case SHAPE_CIRCLE:
                sprintf(coords, "center (%d, %d), radius %d", curr->data.circle.cx, curr->data.circle.cy, curr->data.circle.r);
                printf("%-5d | %-12s | %-35s | '%c'\n", curr->id, "Circle", coords, curr->draw_char);
                break;
            case SHAPE_TRIANGLE:
                sprintf(coords, "v1(%d, %d), v2(%d, %d), v3(%d, %d)", 
                        curr->data.tri.x1, curr->data.tri.y1, 
                        curr->data.tri.x2, curr->data.tri.y2, 
                        curr->data.tri.x3, curr->data.tri.y3);
                printf("%-5d | %-12s | %-35s | '%c'\n", curr->id, "Triangle", coords, curr->draw_char);
                break;
        }
        curr = curr->next;
    }
}

// Add shape prompt
void add_shape() {
    printf("\nSelect Shape Type to Add:\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("Enter choice (1-4): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    if (choice < 1 || choice > 4) {
        printf("Invalid choice.\n");
        return;
    }

    ShapeNode *node = (ShapeNode *)malloc(sizeof(ShapeNode));
    node->id = next_id++;
    node->type = (ShapeType)choice;
    node->draw_char = '*'; // Default drawing character

    int ok = 0;
    switch (node->type) {
        case SHAPE_LINE:
            printf("Enter x1 y1 x2 y2: ");
            if (scanf("%d %d %d %d", &node->data.line.x1, &node->data.line.y1, &node->data.line.x2, &node->data.line.y2) == 4) ok = 1;
            break;
        case SHAPE_RECTANGLE:
            printf("Enter x y (top-left) width height: ");
            if (scanf("%d %d %d %d", &node->data.rect.x, &node->data.rect.y, &node->data.rect.w, &node->data.rect.h) == 4) ok = 1;
            break;
        case SHAPE_CIRCLE:
            printf("Enter cx cy (center) radius: ");
            if (scanf("%d %d %d", &node->data.circle.cx, &node->data.circle.cy, &node->data.circle.r) == 3) ok = 1;
            break;
        case SHAPE_TRIANGLE:
            printf("Enter x1 y1 x2 y2 x3 y3: ");
            if (scanf("%d %d %d %d %d %d", 
                      &node->data.tri.x1, &node->data.tri.y1, 
                      &node->data.tri.x2, &node->data.tri.y2, 
                      &node->data.tri.x3, &node->data.tri.y3) == 6) ok = 1;
            break;
    }

    if (!ok) {
        printf("Invalid shape input.\n");
        while (getchar() != '\n');
        free(node);
        return;
    }

    // Ask if they want to customize the drawing character
    printf("Enter drawing character (press enter for default '*'): ");
    while (getchar() != '\n'); // clear buffer
    char c = getchar();
    if (c != '\n' && c != ' ' && c != '\r') {
        node->draw_char = c;
        while (getchar() != '\n'); // clear rest of input line
    }

    add_shape_node(node);
    printf("Shape added successfully with ID %d!\n", node->id);
}

// Delete shape prompt
void delete_shape() {
    list_shapes();
    if (head == NULL) return;
    printf("Enter ID of shape to delete: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    if (delete_shape_node(id)) {
        printf("Shape %d deleted successfully.\n", id);
    } else {
        printf("Shape with ID %d not found.\n", id);
    }
}

// Modify shape prompt
void modify_shape() {
    list_shapes();
    if (head == NULL) return;
    printf("Enter ID of shape to modify: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    ShapeNode *node = find_shape(id);
    if (node == NULL) {
        printf("Shape with ID %d not found.\n", id);
        return;
    }

    printf("Modifying shape ID %d (%s)...\n", node->id, 
           node->type == SHAPE_LINE ? "Line" : 
           node->type == SHAPE_RECTANGLE ? "Rectangle" : 
           node->type == SHAPE_CIRCLE ? "Circle" : "Triangle");

    int ok = 0;
    switch (node->type) {
        case SHAPE_LINE:
            printf("Current: (%d,%d) to (%d,%d)\n", node->data.line.x1, node->data.line.y1, node->data.line.x2, node->data.line.y2);
            printf("Enter new x1 y1 x2 y2: ");
            if (scanf("%d %d %d %d", &node->data.line.x1, &node->data.line.y1, &node->data.line.x2, &node->data.line.y2) == 4) ok = 1;
            break;
        case SHAPE_RECTANGLE:
            printf("Current: top-left (%d,%d), size %dx%d\n", node->data.rect.x, node->data.rect.y, node->data.rect.w, node->data.rect.h);
            printf("Enter new x y width height: ");
            if (scanf("%d %d %d %d", &node->data.rect.x, &node->data.rect.y, &node->data.rect.w, &node->data.rect.h) == 4) ok = 1;
            break;
        case SHAPE_CIRCLE:
            printf("Current: center (%d,%d), radius %d\n", node->data.circle.cx, node->data.circle.cy, node->data.circle.r);
            printf("Enter new cx cy radius: ");
            if (scanf("%d %d %d", &node->data.circle.cx, &node->data.circle.cy, &node->data.circle.r) == 3) ok = 1;
            break;
        case SHAPE_TRIANGLE:
            printf("Current: (%d,%d), (%d,%d), (%d,%d)\n", 
                   node->data.tri.x1, node->data.tri.y1, 
                   node->data.tri.x2, node->data.tri.y2, 
                   node->data.tri.x3, node->data.tri.y3);
            printf("Enter new x1 y1 x2 y2 x3 y3: ");
            if (scanf("%d %d %d %d %d %d", 
                      &node->data.tri.x1, &node->data.tri.y1, 
                      &node->data.tri.x2, &node->data.tri.y2, 
                      &node->data.tri.x3, &node->data.tri.y3) == 6) ok = 1;
            break;
    }

    if (!ok) {
        printf("Modification failed. Invalid coordinates.\n");
        while (getchar() != '\n');
        return;
    }

    // Modify character
    printf("Enter drawing character (press enter to keep '%c'): ", node->draw_char);
    while (getchar() != '\n');
    char c = getchar();
    if (c != '\n' && c != ' ' && c != '\r') {
        node->draw_char = c;
        while (getchar() != '\n');
    }

    printf("Shape %d modified successfully.\n", node->id);
}

// Z-Order/Layering Management Prompt
void manage_layering() {
    list_shapes();
    if (head == NULL) return;
    printf("\nLayering Options:\n");
    printf("1. Bring Shape to Front (render last, overlaps others)\n");
    printf("2. Send Shape to Back (render first, under others)\n");
    printf("Enter choice (1-2): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    if (choice < 1 || choice > 2) {
        printf("Invalid choice.\n");
        return;
    }

    printf("Enter ID of shape to reorder: ");
    int id;
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    ShapeNode *node = find_shape(id);
    if (node == NULL) {
        printf("Shape %d not found.\n", id);
        return;
    }

    if (choice == 1) {
        bring_to_front(id);
        printf("Shape %d brought to front.\n", id);
    } else {
        send_to_back(id);
        printf("Shape %d sent to back.\n", id);
    }
}

// Customization Menu
void settings_menu() {
    printf("\n=== Canvas Settings ===\n");
    printf("1. Change Canvas Dimensions (Current: %dx%d)\n", canvas_width, canvas_height);
    printf("2. Change Background Character (Current: '%c')\n", bg_char);
    printf("Enter choice (1-2): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    if (choice == 1) {
        int w, h;
        printf("Enter new Width and Height (e.g. 80 24): ");
        if (scanf("%d %d", &w, &h) != 2 || w <= 0 || h <= 0) {
            printf("Invalid dimensions.\n");
            while (getchar() != '\n');
            return;
        }
        free_canvas();
        canvas_width = w;
        canvas_height = h;
        init_canvas();
        printf("Canvas resized to %dx%d successfully.\n", w, h);
    } else if (choice == 2) {
        printf("Enter new background character: ");
        while (getchar() != '\n');
        char c = getchar();
        if (c != '\n' && c != '\r') {
            bg_char = c;
            while (getchar() != '\n');
            printf("Background character updated to '%c'.\n", bg_char);
        } else {
            printf("No character entered.\n");
        }
    }
}

int main() {
    // Read optional dimensions at startup
    printf("Enter custom canvas width and height (or enter 0 0 for default 80x24): ");
    int w, h;
    if (scanf("%d %d", &w, &h) == 2 && w > 0 && h > 0) {
        canvas_width = w;
        canvas_height = h;
    } else {
        printf("Using default dimensions: 80 x 24\n");
        while (getchar() != '\n'); // clear buffer
    }

    init_canvas();

    int choice;
    printf("\n=========================================\n");
    printf("  ASCII 2D Graphics Editor (Linked List)\n");
    printf("=========================================\n");

    while (1) {
        printf("\nMenu:\n");
        printf("1. Add Object\n");
        printf("2. Delete Object\n");
        printf("3. Modify Object\n");
        printf("4. Display Picture\n");
        printf("5. List Objects\n");
        printf("6. Manage Layering (Z-Order)\n");
        printf("7. Canvas/Editor Settings\n");
        printf("8. Exit\n");
        printf("Enter command: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid selection. Please enter a number.\n");
            while (getchar() != '\n'); // clear buffer
            continue;
        }

        switch (choice) {
            case 1:
                add_shape();
                break;
            case 2:
                delete_shape();
                break;
            case 3:
                modify_shape();
                break;
            case 4:
                display_canvas();
                break;
            case 5:
                list_shapes();
                break;
            case 6:
                manage_layering();
                break;
            case 7:
                settings_menu();
                break;
            case 8:
                printf("Freeing resources and exiting...\n");
                free_shapes_list();
                free_canvas();
                return 0;
            default:
                printf("Unknown menu option %d.\n", choice);
        }
    }
}