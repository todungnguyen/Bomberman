#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdbool.h>
#include <termios.h>
#include <errno.h>
#include <poll.h>
#include <dirent.h>
#include <time.h>

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"


typedef struct Carte_info carte_info;
typedef struct Bombe bombe;
typedef struct Personnage personnage;
typedef struct Etat etat;
typedef struct Tab_Etat tab_etat;

//les information de map
struct Carte_info {
    char** carte; //map principale
    int h; //ligne
    int l; //colonne
    char** bonus; //map bonus
};

//caractère de bombe
struct Bombe {
    int vitesse; 
    int portee; //le portée de bombe
    int pose; //pouvoir poser combien de bombe
};

//les information de joueur
struct Personnage {
    char signe; 
    int vie;
    char* nom;
    bombe boom;
};

carte_info load_carte(const char* name);
void affiche_carte(char** tab,int h,int l);
char* affiche_mod();
char* concat(const char* s1, const char* s2);
char* load_mod();
char* load_niveau();
char* affiche_niveau(const char* lien);
void place(carte_info clone, personnage x, int* i, int* j);


/////////////////////////////////////////////////////////////////////////


//liste 
typedef struct block_s block;
typedef block* list;

struct block_s{
    int i;
    int j;
    time_t t;
    block* next;
};

list listvide () {
    list l = NULL;
    return l;
}

list ajout_debut (list l, int i, int j, time_t t){
    block* n = (block*) malloc(sizeof(block));
    n->i = i;
    n->j = j;
    n->t = t;
    n->next = l;
    l = n;
    return l;
}

list supprime_fin (list l) {
    block* b = NULL;
    block* p = NULL;
    block* c = l;

    while (c != NULL) {
        b = p;
        p = c;
        c = c->next;
    }

    if(b != NULL) b->next = NULL;
    else l = listvide();

    return l;
}

//vérifier si (i,j) est dans list l ou non
bool check(list l, int i, int j) {
    block* c = l;

    while (c != NULL) {
        if (c->i == i && c-> j == j) return 1;
        c = c->next;
    }

    return 0;
}

int length (list l) {
    int p = 0;
    block* t = l;
    while( t != NULL) {
        p++;
        t = t->next;
    }
    return p;
}

void affiche(list l){
    block* b = l;
    while(b != NULL ){
        printf("(%d,%d,%ld) ", b->i, b->j,b->t);
        b = b->next;
    }
}


/////////////////////////////////////////////////////////////////////////


//load les infos de map
carte_info load_carte(const char* name) {
    int r;
    char p[1024];

    char buffer[10];
    int cpt = 0;

    int cptLigne = 0;
    carte_info info;

    int fd = open(name, O_RDONLY);

    if(fd == -1) {
        perror(name);
        exit(-1);
    }

    //prendre h (lire jusqu'à quand rencontre un espace)
    while((r=read(fd,p,1))>0 && p[0] != ' ') {
        buffer[cpt] = p[0];
        cpt++;
    }
    buffer[cpt] = '\0';
    cpt = 0;

    info.h = atoi(buffer);

    //prendre l (lire jusqu'à quand rencontre une saute de ligne)
    while((r=read(fd,p,1))>0 && p[0] != '\n') {
        buffer[cpt] = p[0];
        cpt++;
    }
    buffer[cpt] = '\0';
    info.l = atoi(buffer);

    //allouer pour la carte
    info.carte = (char**) calloc(sizeof(char*),info.h);
    for(int i = 0; i < info.h; i ++) {
        info.carte[i] =  calloc(1,info.l+1);
    }

    //prendre la carte (arret à 1er ligne de bonus <=> cptLigne = info.h + 1)
    while((r=read(fd,p,info.l+1))>0 && cptLigne < info.h) {
        strcpy(info.carte[cptLigne],p);
        cptLigne++;
    }

    cptLigne = 0;

    //reculer pointeur
    lseek(fd,-(info.l+1),SEEK_CUR);
    
    //allouer pour bonus
    info.bonus = (char**) calloc(sizeof(char*),info.h);
    for(int i = 0; i < info.h; i ++) {
        info.bonus[i] =  calloc(1,info.l+1);
    }

    //prendre bonus
    while((r=read(fd,p,info.l+1))>0 && cptLigne < info.h) {
        strcpy(info.bonus[cptLigne],p);
        cptLigne++;
    }
    return info;
}

//afficher la carte
void affiche_carte(char** tab,int h,int l) {
    for(int i = 0; i < h; i ++) {
        for(int j = 0; j < l; j ++) {
            if(tab[i][j] == '@' || tab[i][j] == '*' || tab[i][j] == '+') {
                printf(RED "%c" RESET,tab[i][j]);
            } else printf("%c",tab[i][j]);
        }
        printf("\n");
    }
}


/////////////////////////////////////////////////////////////////////////


//affiche les mods possible 
char* affiche_mod() {
    system("clear");
    DIR* d = opendir("mods");
    struct dirent* dirp;

    write(1,"BOMBERMAN\n",11);
    write(1,"Tapez nom de mode pour chosir, q pour quitter\n",47);
    while((dirp = readdir(d)) != NULL) {
        if(dirp->d_type == DT_DIR) {
            if(strcmp(dirp->d_name,".") != 0 && strcmp(dirp->d_name,"..") != 0) {
                write(1,"> ",2);
                write(1,dirp->d_name,strlen(dirp->d_name));
                write(1,"\n",1);
            }
        }
    }
    closedir(d);
    return load_mod();
}

//concatenate 2 char*
char* concat(const char* s1, const char* s2) {
    char* result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

//load un dans les mods
char* load_mod() {
    char mod[1024];
    printf("Mod?: ");
    fgets(mod,1024,stdin);

    //supprimer caractère '\n'
    mod[strlen(mod)-1] = '\0';

    if(strcmp(mod,"q") == 0) exit(1);

    DIR* d = opendir(concat("mods/",mod));
    if(d) return affiche_niveau(concat("mods/",mod));
    else return affiche_mod();
}


////////////////////////////////////////////////////////////////////////////////


//demande choisi un niveau
char* affiche_niveau(const char* lien) {
    system("clear");
    printf("lien: %s\n",lien);
    int r;
    char buffer[1024];

    int fd = open(concat(lien,"/deroulement.txt"),O_RDONLY);

    if(fd == -1) {
        perror("deroulement.txt");
        exit(-1);
    }

    while((r=read(fd,buffer,1024)) > 0) {
        write(1,buffer,r);
    }
    close(fd);
    return load_niveau(lien);
}

//vérfier ce niveau et demader load_carte
char* load_niveau(const char* lien) {
    char niveau[1024];
    printf("Niveau? :");
    fgets(niveau,1024,stdin);

    //supprimer caractère '\n'
    niveau[strlen(niveau)-1] = '\0';

    if(strcmp(niveau,"p") == 0) return affiche_mod();

    char* s = concat(lien,concat("/niveaux/",concat(niveau,".txt")));
    int fd = open(s,O_RDONLY);
    if(fd == -1) return affiche_niveau(lien);
    return s;
}


///////////////////////////////////////////////


//prendre c sans ENTER au lieu de SCANF 
//et NE PAS AFFICHER buffer sur terminal
//update terminal tout le temps
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    //désactivité mode canonique et mode écrire à terminal
    newt.c_lflag&= ~(ICANON | ECHO);

    //Les modifications sont effectuées immédiatement.
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    //update stdin tout le temps
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    //revient aux anciens modes.
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) return ch;

    return 0;
}

//creer un clone de la carte principale
carte_info carte_clone(carte_info info) {
    carte_info clone;
    clone.h = info.h;
    clone.l = info.l;

    clone.carte = (char**) malloc(sizeof(char*)*clone.h);
    for(int i = 0; i < clone.h; i ++) {
        clone.carte[i] =  malloc(clone.l+1);
        strcpy(clone.carte[i],info.carte[i]);
    }

    clone.bonus = (char**) malloc(sizeof(char*)*clone.h);
    for(int i = 0; i < clone.h; i ++) {
        clone.bonus[i] =  malloc(clone.l+1);
        strcpy(clone.bonus[i],info.bonus[i]);
    }

    return clone;
}

/////////////////////////////////////////////////////////////////


//exploser
int explose(int i,int j,carte_info clone, personnage* x, personnage* y) {

    if(clone.carte[i][j] != '0') {
        if(clone.carte[i][j] != ' ' && clone.carte[i][j] != '+' && clone.carte[i][j] != '*' && clone.carte[i][j] != '@' && clone.carte[i][j] != 'x' && clone.carte[i][j] != 'y') {
            if(clone.carte[i][j] != '1') clone.carte[i][j] = clone.carte[i][j] - 1;
            else clone.carte[i][j] = clone.bonus[i][j];
        } else {
            if(clone.carte[i][j] == 'x') {
                if(x->signe == 'x') x->vie --;
                else y->vie --;
            }
            if(clone.carte[i][j] == 'y') {
                if(x->signe == 'y') x->vie --;
                else y->vie --;
            }
        }
    } else return -1; //arret
    return 0;
}

//vérifier si un boom est à l'heure d'exploser ou non
list temp(list etat, carte_info clone, int portee, personnage* x, personnage* y,bool* boom) {
    time_t t;

    //prendre le dernière bombe
    block* p = NULL;
    block* c = etat;
    while (c != NULL) {
        p = c;
        c = c->next;
    }

    time(&t);
    double total_t = difftime(t,p->t);
    if(total_t >= 3) {

        //si on ne trouve pas signe <=> joueur est à place de dernière bombe ajouté (head)
        int i = 0; int j = 0;
        place(clone,*x,&i,&j);
        if(i == 0 && j == 0 && p == etat) {
            clone.carte[p->i][p->j] = x->signe;
            x->vie--;
        }
        else clone.carte[p->i][p->j] = ' ';

        //explose les cas côtés
        for(int a = 1; a <= portee; a++) {
            if(explose(p->i,p->j+a,clone,x,y) == -1) a = portee + 1;
        }

        for(int a = 1; a <= portee; a++) {
            if(explose(p->i,p->j-a,clone,x,y) == -1) a = portee + 1;
        }

        for(int a = 1; a <= portee; a++) {
            if(explose(p->i+a,p->j,clone,x,y) == -1) a = portee + 1;
        }

        for(int a = 1; a <= portee; a++) {
            if(explose(p->i-a,p->j,clone,x,y) == -1) a = portee + 1;
        }

        etat = supprime_fin(etat);
        *boom = 1;
    }

    return etat;
}


/////////////////////////////////////////////////////////////////


//déplacer
int deplace(int i,int j,int i_change,int j_change, personnage* y, carte_info clone, list etat_x, list etat_y) {
    if (clone.carte[i_change][j_change] == ' ' || clone.carte[i_change][j_change] == '+' || clone.carte[i_change][j_change] == '*' || (clone.carte[i_change][j_change] == '@' && (check(etat_x,i_change,j_change) == 0) && (check(etat_y,i_change,j_change) == 0))) {

        //if(clone.carte[i_change][j_change] == '+') y->boom.vitesse ++;
        if(clone.carte[i_change][j_change] == '@') y->boom.pose ++;
        if(clone.carte[i_change][j_change] == '*') y->boom.portee ++;

        clone.carte[i_change][j_change] = y->signe;

        //pour le cas joueur est à même place avec bombe
        if (clone.carte[i][j] != '@') clone.carte[i][j] = ' ';

    } else return -1;
    return 0;
}

//trouver la place à l'instant de joueur
void place(carte_info clone, personnage x, int* i, int* j) {
    for(int a = 0; a < clone.h; a ++) {
        for(int b = 0; b < clone.l; b ++) {
            if (clone.carte[a][b] == x.signe) {
                *i = a; *j = b;
            }
        }
    }
}

//affiche les informations nécessaires
void affiche_info(personnage x, personnage y, carte_info clone) {
    system("clear");
    printf("%s - vie: %d - vitesse: %d - portee: %d - pose %d.\n",x.nom,x.vie,x.boom.vitesse,x.boom.portee,x.boom.pose);
    printf("%s - vie: %d - vitesse: %d - portee: %d - pose %d.\n",y.nom,y.vie,y.boom.vitesse,y.boom.portee,y.boom.pose);
    affiche_carte(clone.carte,clone.h,clone.l);
}

//déplacer, exploser,...
int mouvement(const char* name, personnage x, personnage y) {

    int ix = 0; int jx = 0; int iy = 0; int jy = 0;

    char c;

    //liste des bombes déjà posées de x et y
    list etat_x = listvide(); list etat_y = listvide();

    //s'il y a un boom qui vien d'exploser
    bool boomx = 0; bool boomy = 0;

    time_t t;

    //faire une clone
    carte_info clone = carte_clone(load_carte(name));

    //affiche
    affiche_info(x,y,clone);

    do {
        //chercher la place de x et y
        place(clone,y,&iy,&jy);
        place(clone,x,&ix,&jx);

        c = kbhit();

        //déplacer x et y dépend de la valeur de c
        switch(c) {

            //en haut
            case 'A':
            deplace(iy,jy,iy-y.boom.vitesse,jy,&y,clone,etat_x,etat_y);
            break;

            //à gauche
            case 'D':
            deplace(iy,jy,iy,jy-y.boom.vitesse,&y,clone,etat_x,etat_y);
            break;

            //en bas
            case 'B':
            deplace(iy,jy,iy+y.boom.vitesse,jy,&y,clone,etat_x,etat_y);
            break;

            //à droite
            case 'C':
            deplace(iy,jy,iy,jy+y.boom.vitesse,&y,clone,etat_x,etat_y);
            break;

            //en haut
            case 'z':
            deplace(ix,jx,ix-x.boom.vitesse,jx,&x,clone,etat_x,etat_y);
            break;

            //à gauche
            case 'q':
            deplace(ix,jx,ix,jx-x.boom.vitesse,&x,clone,etat_x,etat_y);
            break;

            //en bas
            case 's':
            deplace(ix,jx,ix+x.boom.vitesse,jx,&x,clone,etat_x,etat_y);
            break;

            //à droite
            case 'd':
            deplace(ix,jx,ix,jx+x.boom.vitesse,&x,clone,etat_x,etat_y);
            break;

            //les autres cas
            default:
            break; 
        }

        //si il y a des booms
        if(length(etat_x) != 0) etat_x = temp(etat_x,clone,x.boom.portee,&x,&y,&boomx);

        if(length(etat_y) != 0) etat_y = temp(etat_y,clone,y.boom.portee,&y,&x,&boomy);

        //poser un boom
        if(c == ' ' && length(etat_x) < x.boom.pose) {
            clone.carte[ix][jx] = '@'; 
            if(check(etat_x,ix,jx) == false) etat_x = ajout_debut(etat_x,ix,jx,time(&t));
        }

        if(c == '\n' && length(etat_y) < y.boom.pose) {
            clone.carte[iy][jy] = '@'; 
            if(check(etat_y,iy,jy) == false) etat_y = ajout_debut(etat_y,iy,jy,time(&t));
        }

        //affiche seulement s'il y a des actions
        if(c != 0 || boomx != 0 || boomy != 0) {
            affiche_info(x,y,clone);
            if(boomx != 0) boomx = 0;
            if(boomy != 0) boomy = 0;
        }

        //chercher winner
        if(x.vie <= 0) {
            printf("%s gagné.\n",y.nom);
            exit(-1);
        }

        if(y.vie <= 0) {
            printf("%s gagné.\n",x.nom);
            exit(-1);
        }

    } while(1);
    return 0;
}

//initialiser un nouveau charactère
personnage initialise(char* name, char signe) {
    bombe bx;
    bx.vitesse = 1;
    bx.portee = 1;
    bx.pose = 1;

    personnage x;
    x.vie = 3;
    x.boom = bx;
    x.nom = name;
    x.signe = signe;

    return x;
}


int main(int argc, char** argv) {

 personnage x = initialise("Joueur 1", 'x');
 personnage y = initialise("Joueur 2", 'y');

 mouvement(affiche_mod(),x,y);

 return 0;
}
















