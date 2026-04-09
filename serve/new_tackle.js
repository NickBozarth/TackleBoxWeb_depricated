const newTackleButton = document.querySelector("#new-tackle-button button");
const newTackleForm = document.getElementById("new-tackle-form");
const tackleTypeSelect = newTackleForm.querySelector("form select");
const tackleDataDiv = newTackleForm.querySelector("form div#data");

var tackleTypes = undefined;



async function processNewTackle() {
    if (passwordHash === undefined || sessionId === undefined) 
        return Result.err("You must be logged in to submit a new tackle");

    let resp1 = await fetchEncrypted("/check_session", { "session_id" : sessionId });
    if (resp1.is_err) return resp1;
    console.log("SUCCESS");

    return Result.ok();
}


async function getTackleSections() {
    return await fetch(
        "/sections",
        {
            method: "GET"
        }
        )
        .then(async response => {
            if (!response.ok) return Result.err("Response returned with code " + response.status);
            try {
                return Result.ok(await response.json());
            } catch {
                return Result.err("Error occurred when parsing response to json");
            }
        });
}


function showCorrectDataField() {
    const selectedOption = tackleTypeSelect.selectedOptions[0];
    if (selectedOption === undefined) return Result.err("No option selected");
    const selectedOptionTypes = selectedOption.classList;
    if (selectedOptionTypes === undefined) return Result.err("Selected option type not found");
    
    for (const child of tackleDataDiv.children) child.hidden = true;
    for (const selectedOptionType of selectedOptionTypes) {
        for (const child of tackleDataDiv.querySelectorAll(`.${selectedOptionType}`)) child.hidden = false;
    }
}


newTackleForm.addEventListener("submit", async event => {
    event.preventDefault();

    console.log(await processNewTackle());
});


newTackleButton.addEventListener("click", async event => {
    if (sessionId === undefined) return;
    if (tackleTypes === undefined) {
        let res = await getTackleSections();
        if (res.is_ok) tackleTypes = res.value;
        else return;
    }

    newTackleButton.hidden = true;
    newTackleForm.hidden = false;

    tackleTypes.forEach(type => {
        let newOption = new Option(type[0], type[0].toLowerCase());
        for(const className of type[1].split(" ")) newOption.classList.add(className);
        tackleTypeSelect.add(newOption); 
    });

    showCorrectDataField();
});

tackleTypeSelect.addEventListener("change", event => {
    showCorrectDataField();
});