Make temporary, working directory. Like so:

mkdir workingdir;

Alias browsertime results directories to

ln -s ../2020-11/browsertime-results-2020-11-09-chrome-1 chrome-current
ln -s ../2020-11/browsertime-results-2020-11-29-fenix-nightly-1 fenix-current

Assume the metrics files to extract are either passed in as arguments
one and two, or set to the defaults.

Defaults:
metrics1 = visual-metrics-2021.txt
metrics2 = web-vitals-2020-edit.txt

To run single URL, single device, single browser visualization, then:

$MOZPERFAX/scripts/browsertime-to-uno-with-1-edit-list.sh

And the generated files will be placed in the nested 'svg' and 'png'
directories, depending on file type.

