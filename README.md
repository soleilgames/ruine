# ruine
Développer mon premier jeu-vidéo en Deux mois sans moteur de jeux, est le défi que je me suis lancé.
Suivez pas à pas mes dév ;) -> [Youtube!](https://www.youtube.com/channel/UCl2HzRATykZnFSPh_i_DNsQ)

# Le game-play (en deux mots)

C'est un mélange entre un jeu d'aventure et un Pac Man 3D.

"Vous connaissez la ruine depuis un moment, mais maintenant vous en êtes sûr ! Il y a un passage vers des oubliettes ... et un trésor caché. Quoi de plus tentant ? Il vous suffit de rentrer prendre le butin et ressortir, ce sera  Veni, Vedi, Profit." 

# Les étapes
## Étape 1
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape1.png)
## Étape 2
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape2.png)
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape2bis.png)
## Étape 3
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape3.png)
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape3bis.png)
## Étape 4 Finale
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape4.png)
![Image of Ruine](https://raw.githubusercontent.com/soleilgames/ruine/master/presse/etape4editeur.png)


---

# Developper Guide

## Code Guidelines

 * The code is formatted by clang-format: https://raw.githubusercontent.com/soleilgames/ruine/master/.clang-format
 * Currently it's unsure whereas OOP or Data Oriented Programming will be chosen
 * Code must be compliant with GLES 2.0 (Yay!!!)
 * NO game engine. Libraries are allowed for trivial tasks (mathematics, decoding PNG, ...)


## Compile for Android

Android is the target for this game. To build it, you need a recent version of the NDK and SDK installed.

```
$ cd ruine/android
$ cat local.properties
ndk.dir=/opt/android/android-ndk-r15c
sdk.dir=/home/florian/Android/Sdk
$ ./gradlew build					# Build the application
$ adb install -r app/build/outputs/apk/app-debug.apk	# Install the app
$ adb logcat | tee tmp.txt | grep soleil		# Just to follow logs
```


## Compile for GNU/Linux

The game is not intended to run on desktop however it can help for development (there is currently no sound support).
You will need the GLFW 3 and Cmake:

```
cd ruine
mkdir build
cd build
cmake ../
make
```


# License
  L'application est sous license MIT.
  La musique a été composé par M. Horacio M. Goldberg (www.yosoygames.com.ar/royalty-free)
  Le fantôme par Antoine Tallon (http://www.at-graphisme.fr/category/travauxperso/game-design/)
  La pièce et la clé sont d'après le travail de yd (https://opengameart.org/users/yd)
  La texture de mur d'après le travail rubberduck (https://opengameart.org/users/rubberduck)