if [ ! -d "external" ]; then
  mkdir external
  sleep 0.1
fi

if [ ! -d "external/acpica" ]; then
	cd external
	git clone https://github.com/acpica/acpica.git
	cd ../
fi

