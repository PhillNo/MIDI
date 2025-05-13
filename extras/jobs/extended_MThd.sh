test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/extended_MThd.mid ${test_dir}/encoded_files/extended_MThd.mid > ${test_dir}/results/extended_MThd.txt

result=$(head -n 1 ${test_dir}/results/extended_MThd.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

