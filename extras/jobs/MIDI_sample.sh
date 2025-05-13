test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/MIDI_sample.mid ${test_dir}/encoded_files/MIDI_sample.mid > ${test_dir}/results/MIDI_sample.txt

result=$(head -n 1 ${test_dir}/results/MIDI_sample.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

