test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/bad_sysex.mid ${test_dir}/encoded_files/bad_sysex.mid > ${test_dir}/results/bad_sysex.txt

result=$(head -n 1 ${test_dir}/results/bad_sysex.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

