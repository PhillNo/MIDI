test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/no_track.mid ${test_dir}/encoded_files/no_track.mid > ${test_dir}/results/no_track.txt

result=$(head -n 1 ${test_dir}/results/no_track.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

