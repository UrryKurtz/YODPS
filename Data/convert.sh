for file in *.obj; do
base=${file%.*}
mdl=${base}.mdl

echo " TEST "${file} ${base} ${mdl}
./AssetImporter model ${file} Models/${mdl} -t 
# -l

done