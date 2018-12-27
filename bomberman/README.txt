PROJET BOMBERMAN

NGUYEN To Dung - CHU Viet Dung

Nous avons fait jusqu'à phase 3.
 
- La principale de notre projet est des structures qui aide à stocker tous les informations nécessaires et les transforme entre les fonctions.

+ Point positive:
Phase 1:
- Tous d'abord, nous avons chargé mod par les fonctions de répertoire (chdir,etc) et niveau par appel système (open, close,etc).

- Pour stocker un map, il y a struct carte_info.

Phase 2:
- Pour l'entrée au clavier ne soit pas affiché et mettre à jour l'écran dans le temps, il y a termios et fcntl() dans la fonction kbhit().

Phase 3:
- Un bombe va exploser après 3s, pour cela, nous avons choisi time.h.
- Nous avons utiliser aussi la liste chainée pour stocker les bombes (pour distinguer @ de bombe et @ de powerups).
- Coloré les powerups par \x1B[31m,etc


+ Point négative:
- La plupart du code est if else et beaucoup de variable, donc le code n'a pas assez de lisible.
- On n'a aucune idée de augmenter la vitesse, donc le + de power ups ne marche pas.

Nous avons décidé que c'est moi (To Dung) qui va coder et après mon camarade (Viet Dung) va vérifier et réduire le code.
