test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/sample.mid ${test_dir}/encoded_files/sample.mid > ${test_dir}/results/sample.txt

result=$(head -n 1 ${test_dir}/results/sample.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

