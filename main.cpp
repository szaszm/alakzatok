#include "SDL.h"
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <vector>

struct Pont {
	int x;
	int y;

	Pont() :x(0), y(0) {}
	Pont(int x, int y) :x(x), y(y) {}

	Pont& operator+=(Pont p) {
		x += p.x;
		y += p.y;
		return *this;
	}
};

Pont operator+(const Pont& a, const Pont& b) {
	return Pont(a.x + b.x, a.y + b.y);
}

int tavolsag(const Pont& a, const Pont& b) {
	return static_cast<int>(sqrt((b.x-a.x)*(b.x-a.x) + (b.y - a.y)*(b.y-a.y)));
}

uint32_t szin_invertal(uint32_t regiszin) {
	return regiszin ^ 0xFFFFFF00;
}

uint8_t szin_piros(uint32_t szin) { return szin >> 24; }
uint8_t szin_zold(uint32_t szin) { return szin >> 16 & 0xFF; }
uint8_t szin_kek(uint32_t szin) { return szin >> 8 & 0xFF; }
uint8_t szin_atlatszatlansag(uint32_t szin) { return szin & 0xFF; }


class Alakzat {
	uint32_t szin;
public:
	explicit Alakzat(uint32_t szin) :szin(szin) {}
	virtual ~Alakzat() {}

	uint32_t get_szin() const { return szin; }
	void set_szin(uint32_t ujszin) { szin = ujszin; }

	virtual void rajzol(SDL_Renderer*) const = 0;
	virtual void kiir() const {
		std::cout << "Alakzat::kiir()\n";
	}
	virtual bool bennevan(Pont p) const = 0;
	virtual void mozgat(Pont ennyivel) = 0;
};

void invertal(Alakzat& alakzat) {
	const uint32_t regiszin = alakzat.get_szin();
	const uint32_t ujszin = szin_invertal(regiszin);
	alakzat.set_szin(ujszin);
}

void kiir(Alakzat const& a) {
	a.kiir();
}

class Kor : public Alakzat {
	Pont kozeppont;
	int sugar;
public:
	Kor(uint32_t szin, Pont kozeppont, int sugar)
		:Alakzat(szin), kozeppont(kozeppont), sugar(sugar)
	{}

	void rajzol(SDL_Renderer* const renderer) const override {
		const uint32_t szin = get_szin();
		const int darabok_szama = 16;
		SDL_Vertex csucsok[darabok_szama];
		for (int i = 0; i < darabok_szama; ++i) {
			const double szog = i * 2 * M_PI / darabok_szama;
			const Pont eltolas(
				static_cast<int>(sugar * cos(szog)),
				static_cast<int>(sugar * sin(szog))
			);
			const Pont eltolt_pont = kozeppont + eltolas;
			csucsok[i].position.x = eltolt_pont.x;
			csucsok[i].position.y = eltolt_pont.y;
			csucsok[i].color.r = szin_piros(szin);
			csucsok[i].color.g = szin_zold(szin);
			csucsok[i].color.b = szin_kek(szin);
			csucsok[i].color.a = szin_atlatszatlansag(szin);
		}
		int indexek[(darabok_szama - 2) * 3];
		for (int i = 0; i < darabok_szama - 2; ++i) {
			indexek[i * 3] = 0;
			indexek[i * 3 + 1] = i + 1;
			indexek[i * 3 + 2] = i + 2;
		}
		SDL_RenderGeometry(renderer, NULL, csucsok, sizeof(csucsok)/sizeof(SDL_Vertex), indexek, sizeof(indexek)/sizeof(int));
	}

	void kiir() const override {
		std::cout << "Kor::kiir()\n";
	}

	bool bennevan(Pont p) const override {
		return tavolsag(p, kozeppont) <= sugar;
	}

	void mozgat(Pont ennyivel) override {
		kozeppont += ennyivel;
	}
};

class Teglalap : public Alakzat {
	Pont balfelso;
	int szelesseg;
	int magassag;
public:
	Teglalap(uint32_t szin, Pont balfelso, int szelesseg, int magassag)
		:Alakzat(szin), balfelso(balfelso), szelesseg(szelesseg), magassag(magassag)
	{}

	void rajzol(SDL_Renderer* const renderer) const override {
		const uint32_t szin = get_szin();
		SDL_Vertex csucsok[4];
		for (int i = 0; i < 4; ++i) {
			csucsok[i].color.r = szin_piros(szin);
			csucsok[i].color.g = szin_zold(szin);
			csucsok[i].color.b = szin_kek(szin);
			csucsok[i].color.a = szin_atlatszatlansag(szin);
		}
		csucsok[0].position.x = balfelso.x;
		csucsok[0].position.y = balfelso.y;
		csucsok[1].position.x = balfelso.x + szelesseg;
		csucsok[1].position.y = balfelso.y;
		csucsok[2].position.x = balfelso.x;
		csucsok[2].position.y = balfelso.y + magassag;
		csucsok[3].position.x = balfelso.x + szelesseg;
		csucsok[3].position.y = balfelso.y + magassag;
		int indexek[] = {0, 1, 2, 0, 2, 3};
		SDL_RenderGeometry(renderer, NULL, csucsok, 4, indexek, 6);
	}

	bool bennevan(Pont p) const override {
		return p.x >= balfelso.x && p.y >= balfelso.y && p.x <= (balfelso.x + szelesseg) && p.y <= (balfelso.y + magassag);
	}

	void mozgat(Pont ennyivel) override {
		balfelso += ennyivel;
	}
};

int main(int argc, char** argv) {
	// SDL inicializálása, siker esetén kilépés előtt SDL_Quit-et kell hívni
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Nem sikerült inicializálni az SDL-t: %s\n", SDL_GetError());
		return 1;
	}

	// SDL ablak nyitása, be kell zárni az SDL_DestroyWindow hívással
	SDL_Window* const ablak = SDL_CreateWindow("SDL példa", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 450, SDL_WINDOW_SHOWN);
	if (!ablak) {
		fprintf(stderr, "Nem sikerült SDL ablakot nyitni: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Várjunk, mert a rendszeremen nem jelentek meg az azonnal rajzolt dolgok
	SDL_Delay(100);

	SDL_Renderer* const renderer = SDL_CreateRenderer(ablak, -1, SDL_RENDERER_SOFTWARE);

	// Elso pelda, 16. dia
	if (false) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		Teglalap t1(0xFF0000FF, Pont(500, 250), 250, 150);
		Kor k1(0x00FF00FF, Pont(400, 200), 150);

		std::vector<Alakzat*> alakzatok;
		alakzatok.push_back(&t1);
		alakzatok.push_back(&k1);

		for (size_t i = 0; i < alakzatok.size(); ++i) {
			alakzatok[i]->rajzol(renderer);
		}

		SDL_RenderPresent(renderer);
		SDL_Delay( 3000);
	}

	// Masodik pelda, 19. dia
	if (false) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		std::vector<Alakzat*> alakzatok; /* tulajdonos: majd törölnie kell! */

		alakzatok.push_back(new Teglalap(0xFF0000FF, Pont(400, 50), 350, 250));
		alakzatok.push_back(new Kor(0x00FF00FF, Pont(250, 300), 200));

		for (size_t i = 0; i < alakzatok.size(); ++i)
			alakzatok[i]->rajzol(renderer);

		for (size_t i = 0; i < alakzatok.size(); ++i)
			delete alakzatok[i]; /* destruktort hív */

		SDL_RenderPresent(renderer);
		SDL_Delay( 3000);
	}

	// Harmadik pelda, 20-22. dia
	{
		std::vector<Alakzat*> alakzatok = {
				new Teglalap(0xFF0000FF, Pont(400, 50), 350, 250),
				new Kor(0x00FF00FF, Pont(250, 300), 200)
		};
		SDL_Event ev;
		Alakzat* mozgatott = NULL;
		while (SDL_WaitEvent(&ev) && ev.type != SDL_QUIT) {
			bool mozgott = false;
			switch (ev.type) {
				case SDL_MOUSEBUTTONDOWN: {
					for (int i = alakzatok.size()-1; i>=0; --i) {
						Pont hol(ev.button.x, ev.button.y);
						if (alakzatok[i]->bennevan(hol)) { /* bennevan() */
							mozgatott = alakzatok[i];
							break; /* első után álljunk meg */
						}
					}
				}
				break;
				case SDL_MOUSEBUTTONUP: {
					if (ev.button.button==SDL_BUTTON_LEFT) {
						mozgatott = NULL;
					}
				}
				break;
				case SDL_MOUSEMOTION: {
					Pont mennyivel(ev.motion.xrel, ev.motion.yrel);
					if (mozgatott) {
						mozgatott->mozgat(mennyivel);
						mozgott = true;
					}
				}
				break;
			}

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(renderer);

			for (const auto& alakzat : alakzatok) {
				alakzat->rajzol(renderer);
			}

			SDL_RenderPresent(renderer);
		}

		for (size_t i = 0; i < alakzatok.size(); ++i) {
			delete alakzatok[i];
		}
	}

	SDL_DestroyWindow(ablak);
	SDL_Quit();
}
