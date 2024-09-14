package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"strings"
)

func main() {
	files := getFiles()
	println("-------------")
	for _, f := range files {
		res := run(f)
		if getRes(res, f) {
			println("\033[32msuccess:\033[0m test passed in", f)
		}
		println("-------------")
	}
}

func getFiles() []string {
	files := make([]string, 0)
	f, err := os.ReadDir("./")
	if err != nil {
		log.Fatal(err)
	}

	for _, file := range f {
		if !file.IsDir() {
			dot := strings.Split(file.Name(), ".")
			if len(dot) < 2 {
				continue
			}
			if dot[1] != "ghoul" {
				continue
			}
			files = append(files, file.Name())
			continue
		}
		subFiles, err := os.ReadDir(fmt.Sprintf("./%s", file.Name()))
		if err != nil {
			log.Fatal(err)
		}

		for _, subFile := range subFiles {
			dot := strings.Split(subFile.Name(), ".")
			if dot[1] != "ghoul" {
				continue
			}
			files = append(files, fmt.Sprintf("%s/%s", file.Name(), subFile.Name()))
		}
	}
	return files
}

func run(filepath string) string {
	path, err := exec.LookPath("./ghoul")
	if err != nil {
		log.Fatal(err)
	}

	cmd := exec.Command(path, filepath)
	out, err := cmd.CombinedOutput()
	if err != nil {
		println(fmt.Sprintf("\033[31merror:\033[0m non zero exit, code %s;\n %s", err, out))
		return ""
	}
	outstring := string(out)
	return outstring
}

func getRes(fileData string, fileName string) bool {
	data := strings.Split(fileData, "\n")
	data = data[:len(data)-1]
	expected := make([]string, 0)
	actual := make([]string, 0)
	expectedFound := false
	actualFound := false
	for _, line := range data {
		if line == "$expected$" {
			expectedFound = true
			continue
		}
		if line != "$expected$" && !expectedFound {
			continue
		}
		if line == "$actual$" {
			actualFound = true
			continue
		}
		if actualFound && expectedFound {
			actual = append(actual, line)
			continue
		}
		if expectedFound && !actualFound {
			expected = append(expected, line)
			continue
		}
	}
	if !expectedFound {
		println(fmt.Sprintf("\033[31mtest failed:\033[0m no $expected$ in %s", fileName))
		return false
	}
	if !actualFound {
		println(fmt.Sprintf("\033[31mtest failed:\033[0m no $actual$ in %s", fileName))
		return false
	}

	if len(expected) != len(actual) {
		println(fmt.Sprintf("\033[31mtest failed:\033[0m expected length is not equal to actual length in %s; expected is %d, actual is %d", fileName, len(expected), len(actual)))
		return false
	}

	anyWrong := false
	for i := range expected {
		if expected[i] != actual[i] {
			println(fmt.Sprintf("\033[31mtest failed:\033[0m test failled in %s; expected: \033[32m%s\033[0m; but got: \033[31m%s\033[0m", fileName, expected[i], actual[i]))
			anyWrong = true
		}
	}
	if anyWrong {
		return false
	}

	return true
}
