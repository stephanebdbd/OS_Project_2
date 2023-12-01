#!/bin/bash

declare -i test_num=1
declare -i test_success=0

function test_new_images() {
   echo -e "\n === Tests de l'option -a avec de nouvelles images ===\n"
   
   while read -r line; do
      values=($line)
      result=$(cd ..; ./pokedex-client <<< "test/img/${values[0]}.bmp" | tail -n 1)
      expected_result="Most similar image found: 'img/${values[1]}.bmp' with a distance of ${values[2]}."
      
      echo -n "Test #$test_num  --  ./pokedex-client <<< test/img/${values[0]}.bmp ... "
      
      if [ ! "$result" = "$expected_result" ]; then
         echo -e '\x1B[0;31mÉchec\x1B[0m'
         echo -e "Résultat obtenu :\n\t\x1B[0;33m$result\x1B[0m\nRésultat attendu :\n\t\x1B[0;33m$expected_result\x1B[0m"
      else
         echo -e '\x1B[0;32mSuccès\x1B[0m'
         test_success+=1
      fi
      test_num+=1
   done < test-new-images.data
   
}

function test_new_images_all_in_one() {
   echo -e "\n === Tests plusieurs images pour un client ===\n"
   
   
   expected_result=""

   result="$(
   count_images=( $(wc -l test-new-images.data) );
   while read -r line; do
         values=($line)
         echo "img/${values[0]}.bmp";
      done < test-new-images.data | ../pokedex-client | tail -n $count_images
      )"

   expected_result="$(while read -r line; do
         values=($line)
         echo "Most similar image found: 'img/${values[1]}.bmp' with a distance of ${values[2]}."
      done < test-new-images.data
      )"
   
   echo -n "Test #$test_num ... "
   
   if [ ! "$result" = "$expected_result" ]; then
      echo -e '\x1B[0;31mÉchec\x1B[0m'
      echo -e "Résultat obtenu :\n\t\x1B[0;33m$result\x1B[0m\nRésultat attendu :\n\t\x1B[0;33m$expected_result\x1B[0m"
   else
      echo -e '\x1B[0;32mSuccès\x1B[0m'
      test_success+=1
   fi
   test_num+=1
   
}

function test_invalid_path() {
   echo -e "\n === Tests avec un mauvais chemin/aucune image. ===\n"
   
   result=$(cd ..; echo -n -e '\n' | ./pokedex-client | tail -n 1)
   expected_result="No similar image found (no comparison could be performed successfully)."
   
   echo -n "Test #$test_num  -- echo -n -e '\n' | ./pokedex-client ... "
   
   if [ ! "$result" = "$expected_result" ]; then
      echo -e '\x1B[0;31mÉchec\x1B[0m'
      echo -e "Résultat obtenu :\n\t\x1B[0;33m$result\x1B[0m\nRésultat attendu :\n\t\x1B[0;33m$expected_result\x1B[0m"
   else
      echo -e '\x1B[0;32mSuccès\x1B[0m'
      test_success+=1
   fi
   test_num+=1
   
   result=$(cd ..; echo -n -e '' | ./pokedex-client | tail -n 1)
   expected_result=""
   
   echo -n "Test #$test_num  -- echo -n -e '' | ./pokedex-client ... "
   
   if [ ! "$result" = "$expected_result" ]; then
      echo -e '\x1B[0;31mÉchec\x1B[0m'
      echo -e "Résultat obtenu :\n\t\x1B[0;33m$result\x1B[0m\nRésultat attendu :\n\t\x1B[0;33m$expected_result\x1B[0m"
   else
      echo -e '\x1B[0;32mSuccès\x1B[0m'
      test_success+=1
   fi
   test_num+=1
}

echo "Fin des img-search en cours."
killall img-search 2>/dev/null

echo "Lancement du serveur en arrière plan."
$(cd ..; ./img-search ) &>/dev/null &

echo "Attente de 10 sec avant le lancement des clients."
for i in {10..1}; do
   echo -n -e "\rTemps restant : $i sec "
   sleep 1
done

test_invalid_path

test_new_images

test_new_images_all_in_one

echo "Fin des img-search en cours."
killall img-search

test_num=$(( test_num - 1 ))
echo -e "\nRésultat final : $test_success/$test_num"
